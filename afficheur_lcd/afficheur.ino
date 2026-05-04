#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mysql/mysql.h>
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>

int configurerPortSerie(struct termios *save);

int main() {

    struct termios save;
    int fd = configurerPortSerie(&save);

    while (1) {

        MYSQL *conn = mysql_init(NULL);

        mysql_real_connect(conn, "172.16.1.4", "AdminGestion", "bonjour", "sgdb", 0, NULL, 0);

        mysql_query(conn,
            "SELECT Temp, Dir, Hum, Pres, `Vitesse de vent`, `Total des précipitations` "
            "FROM meteo_sgdb ORDER BY ID DESC LIMIT 1"
        );

        MYSQL_RES *res = mysql_store_result(conn);
        MYSQL_ROW row = mysql_fetch_row(res);

        if (row) {

            float vent = atof(row[4]) / 3;

            char buffer[128];

            snprintf(buffer, sizeof(buffer),
                "_01Z00_02AA_1B_30_62_1C2Temperature : %s°C_04", row[0]);
            write(fd, buffer, strlen(buffer));
            sleep(2);

            snprintf(buffer, sizeof(buffer),
                "_01Z00_02AA_1B_30_62_1C2Direction : %s_04", row[1]);
            write(fd, buffer, strlen(buffer));
            sleep(2);

            snprintf(buffer, sizeof(buffer),
                "_01Z00_02AA_1B_30_62_1C2Humidite : %s%%_04", row[2]);
            write(fd, buffer, strlen(buffer));
            sleep(2);

            snprintf(buffer, sizeof(buffer),
                "_01Z00_02AA_1B_30_62_1C2Pression : %s hPa_04", row[3]);
            write(fd, buffer, strlen(buffer));
            sleep(2);

            snprintf(buffer, sizeof(buffer),
                "_01Z00_02AA_1B_30_62_1C2Vent : %.1f km/h_04", vent);
            write(fd, buffer, strlen(buffer));
            sleep(2);

            snprintf(buffer, sizeof(buffer),
                "_01Z00_02AA_1B_30_62_1C2Pluie : %s mm_04", row[5]);
            write(fd, buffer, strlen(buffer));
            sleep(2);
        }

        mysql_free_result(res);
        mysql_close(conn);

        sleep(2);
    }
}

int configurerPortSerie(struct termios *save) {

    int fd = open("/dev/ttyUSB0", O_RDWR | O_NOCTTY);

    struct termios cfg;
    tcgetattr(fd, save);
    tcgetattr(fd, &cfg);

    cfsetispeed(&cfg, B9600);
    cfsetospeed(&cfg, B9600);

    cfg.c_cflag = (CLOCAL | CREAD | CS8);
    cfg.c_lflag = 0;
    cfg.c_iflag = 0;
    cfg.c_oflag = 0;

    tcsetattr(fd, TCSANOW, &cfg);

    return fd;
}