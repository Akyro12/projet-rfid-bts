#include <stdio.h>             // Pour les fonctions d'entrée/sortie
#include <sys/types.h>         // Types de base
#include <termios.h>           // Configuration du port série
#include <fcntl.h>             // Contrôle des fichiers (open, etc.)
#include <stdlib.h>            // Fonctions utilitaires (exit, malloc…)
#include <unistd.h>            // Fonctions UNIX (read, write…)
#include <string.h>            // Fonctions de manipulation de chaînes
#include <mysql/mysql.h>       // Librairie MySQL

#define SERIAL_DEVICE "/dev/ttyUSB0"  // Chemin du port série (modifie si besoin)

// Liste de tags valides (ancienne méthode locale, non utilisée ici)
const char valideBadges[3][11] = {
    "12004E200E", // Badge bleu
    "05008185EA", // Badge rouge
    "0500818563"  // Badge jaune
};

// Fonction qui vérifie si le tag est dans la base de données MySQL
int is_tag_in_database(const char* tag) {
    MYSQL *conn;
    MYSQL_RES *res;
    int tagValide = 0;

    conn = mysql_init(NULL);  // Initialisation de la connexion MySQL
    if (conn == NULL) {
        fprintf(stderr, "Erreur mysql_init()\n");
        return 0;
    }

    // Connexion à la base distante (adresse IP, utilisateur, mot de passe, base)
    if (mysql_real_connect(conn, "172.16.1.4", "AdminGestion", "bonjour", "sgdb", 3306, NULL, 0) == NULL) {
        fprintf(stderr, "Erreur connexion MySQL : %s\n", mysql_error(conn));
        mysql_close(conn);
        return 0;
    }

    // Requête SQL pour chercher le tag
    char query[256];
    snprintf(query, sizeof(query), "SELECT * FROM log_acces WHERE Tag_RFID = '%s'", tag);

    // Exécution de la requête
    if (mysql_query(conn, query)) {
        fprintf(stderr, "Erreur requête SQL : %s\n", mysql_error(conn));
        mysql_close(conn);
        return 0;
    }

    res = mysql_store_result(conn);  // Récupération du résultat
    if (res == NULL) {
        fprintf(stderr, "Erreur mysql_store_result() : %s\n", mysql_error(conn));
        mysql_close(conn);
        return 0;
    }

    // Si au moins une ligne retournée, alors le tag est valide
    if (mysql_num_rows(res) > 0) {
        tagValide = 1;
    }

    // Nettoyage
    mysql_free_result(res);
    mysql_close(conn);
    return tagValide;
}

int main() {
    struct termios serial_port_settings;
    int fd;
    int retval;
    char buf[256];
    char tag[20];
    unsigned char nombre[3];
    int n, i, j;

    // Ouverture du port série
    fd = open(SERIAL_DEVICE, O_RDWR);
    if (fd < 0) {
        perror("Échec ouverture port série");
        exit(1);
    }

    // Lecture de la configuration actuelle
    retval = tcgetattr(fd, &serial_port_settings);
    if (retval < 0) {
        perror("Échec récupération configuration série");
        exit(2);
    }

    // Configuration de la vitesse à 9600 bauds
    cfsetospeed(&serial_port_settings, B9600);
    cfsetispeed(&serial_port_settings, B9600);

    // Configuration des bits (8 bits, pas de parité, 1 stop)
    serial_port_settings.c_cflag &= ~PARENB;
    serial_port_settings.c_cflag &= ~CSTOPB;
    serial_port_settings.c_cflag &= ~CSIZE;
    serial_port_settings.c_cflag |= CS8;

    // Activer lecture série
    serial_port_settings.c_cflag |= (CLOCAL | CREAD);

    // Mode "raw" (brut) sans traitement de la ligne
    serial_port_settings.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
    serial_port_settings.c_iflag &= ~(IXON | IXOFF | IXANY);
    serial_port_settings.c_oflag &= ~OPOST;

    // Appliquer la configuration
    tcsetattr(fd, TCSANOW, &serial_port_settings);

    printf("Port série configuré. Attente d’un badge RFID...\n");

    // Boucle infinie : lecture en continu des badges
    while (1) {
        // Lecture des données envoyées par l'Arduino (le tag brut)
        retval = read(fd, (char *)buf, sizeof(buf));
        if (retval < 0) {
            perror("Erreur lecture série");
            continue;
        }

        printf("%d octets lus : %s\n", retval, buf);

        // Transformation du tag depuis le format hexa en ASCII
        for (i = 0, j = 1; j < (retval - 7); j += 2, i++) {
            nombre[0] = buf[j];
            nombre[1] = buf[j+1];
            nombre[2] = '\0';
            sscanf((char *)nombre, "%X", &n);
            tag[i] = (unsigned char)n;
        }
        tag[i] = '\0';

        printf("Tag extrait : %s\n", tag);

        // Envoi de la réponse à l'Arduino
        char message[64];
        if (is_tag_in_database(tag)) {
            printf("Tag autorisé\n");
            strcpy(message, "oui\n");
        } else {
            printf("Tag non reconnu\n");
            strcpy(message, "non\n");
        }

        write(fd, message, strlen(message));
        printf("Réponse envoyée : %s", message);

        // Pause avant nouvelle lecture
        sleep(1);
    }

    close(fd);
    return 0;
}
