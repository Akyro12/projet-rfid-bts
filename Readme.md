README.md — Projet RFID + Supervision Météo (BTS CIEL)
 Présentation du projet

Ce projet consiste en un système embarqué distribué basé sur un Arduino Mega et un Raspberry Pi, permettant :

Le contrôle d’accès par badge RFID
La validation des utilisateurs via une base de données MySQL
L’affichage d’informations météo sur un journal lumineux
La communication entre plusieurs systèmes via liaison série (UART / RS232)

L’objectif est de mettre en œuvre une architecture complète combinant embarqué, réseau local et base de données.

 Architecture globale

Le système est divisé en trois parties principales :

 1 Arduino Mega (Système RFID et actionneur)

Rôle :

Lecture des badges RFID
Envoi du tag au Raspberry Pi
Réception de la réponse (oui/non)
Affichage sur LCD
Activation d’un relais (porte / gâche électrique)

Communication :

RFID → Serial1
Raspberry Pi ->  Serial3
 2 Raspberry Pi -> Module RFID (Validation accès)

Rôle :

Réception du tag RFID envoyé par l’Arduino
Conversion du format brut en UID lisible
Vérification dans une base de données MySQL
Retour d’une réponse :
oui -> accès autorisé
non -> accès refusé

Technologies :

C (Linux)
MySQL C API
Communication série /dev/ttyUSB0

 3 Raspberry Pi — Module journal lumineux (Météo)

Rôle :

Lecture des données météo depuis une base MySQL
Récupération des dernières valeurs :
Température
Humidité
Pression
Vent
Pluie
Envoi vers un affichage externe via RS232

Protocole utilisé :

Format propriétaire du journal lumineux :

_01Z00_..._04

 Fonctionnement global
Un badge RFID est scanné sur l’Arduino
L’Arduino envoie le tag au Raspberry Pi
Le Raspberry Pi vérifie le badge dans la base MySQL
Il renvoie :
oui -> accès autorisé
non -> accès refusé
L’Arduino :
affiche le résultat sur LCD
active ou non le relais
En parallèle, le second programme Raspberry Pi affiche les données météo sur un journal lumineux

Technologies utilisées
Arduino Mega
Raspberry Pi (Linux)
C (système embarqué)
MySQL
UART / RS232
LCD 16x2
RFID RC522 (ou équivalent)
Relais électromagnétique

 Structure du projet
projet-rfid-bts/
|
|-- arduino/
|   |-- rfid_lcd_relay.ino
|
|-- raspberry/
|   |-- rfid_validation.c
|   |-- journal_meteo.c
|
|-- README.md

 Objectifs pédagogiques

Ce projet permet de mettre en œuvre :

Communication série entre systèmes hétérogènes
Lecture de capteurs RFID
Interaction avec une base de données MySQL
Programmation bas niveau en C sous Linux
Gestion d’actionneurs (relais)
Interface utilisateur (LCD)
Système distribué embarqué

 Points techniques importants
La communication Arduino <->Raspberry Pi est réalisée via UART
Le format de transmission RFID est converti en UID ASCII
Les réponses du Raspberry Pi sont simplifiées (oui / non)
Le journal lumineux utilise un protocole série propriétaire

 Améliorations possibles
Passage à un protocole JSON pour la communication
Centralisation complète de la logique sur Raspberry Pi
Ajout d’une interface web de supervision
Sécurisation des requêtes SQL (requêtes préparées)
Ajout de logs système

 Auteur

Projet réalisé dans le cadre du BTS CIEL
Système embarqué — contrôle d’accès RFID + supervision météo