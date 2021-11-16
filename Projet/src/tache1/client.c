/*
 * SPDX-FileCopyrightText: 2021 John Samuel
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include "client.h"
#include "bmp.h"

void analyse(char *pathname, char *data)
{
    //compte de couleurs
    couleur_compteur *cc = analyse_bmp_image(pathname);

    int count;
    strcpy(data, "couleurs: ");
    char temp_string[10] = "10,";
    if (cc->size < 10)
    {
        sprintf(temp_string, "%d,", cc->size);
    }
    strcat(data, temp_string);

    //choisir 10 couleurs
    for (count = 1; count < 11 && cc->size - count > 0; count++)
    {
        if (cc->compte_bit == BITS32)
        {
            sprintf(temp_string, "#%02x%02x%02x,", cc->cc.cc24[cc->size - count].c.rouge, cc->cc.cc32[cc->size - count].c.vert, cc->cc.cc32[cc->size - count].c.bleu);
        }
        if (cc->compte_bit == BITS24)
        {
            sprintf(temp_string, "#%02x%02x%02x,", cc->cc.cc32[cc->size - count].c.rouge, cc->cc.cc32[cc->size - count].c.vert, cc->cc.cc32[cc->size - count].c.bleu);
        }
        strcat(data, temp_string);
    }

    //enlever le dernier virgule
    data[strlen(data) - 1] = '\0';
}

/* 
 * Fonction d'envoi et de réception de messages
 * Il faut un argument : l'identifiant de la socket
 */

int envoie_recois_message(int socketfd, char* data, char* code)
{
    // Demander à l'utilisateur d'entrer un message
    char message[100];
    fgets(message, 1024, stdin);
    strcpy(data, code);
    strcat(data, message);
    return 0;
}

//envoie le nom du client au serveur
int envoie_nom_de_client(int socketfd, char* data)
{
    char nomClient[256];
    gethostname(nomClient, 256);
    strcpy(data, "nom: ");
    strcat(data, nomClient);
    return 0;
}

int main(int argc, char **argv)
{
    int socketfd;
    int bind_status;

    struct sockaddr_in server_addr, client_addr;

    /*
   * Creation d'une socket
   */
    socketfd = socket(AF_INET, SOCK_STREAM, 0);
    if (socketfd < 0)
    {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    //détails du serveur (adresse et port)
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    //demande de connection au serveur
    int connect_status = connect(socketfd, (struct sockaddr *)&server_addr, sizeof(server_addr));
    if (connect_status < 0)
    {
        perror("connection serveur");
        exit(EXIT_FAILURE);
    }

    char data[1024];
    // la réinitialisation de l'ensemble des données
    memset(data, 0, sizeof(data));
    
    char fonction[20];
    int done = 0;
    while (done != 1)
    {
        printf("Lancer une fonction : \n 'message' \n 'nom' (Récupérer le nom du client) \n 'calcul' \n 'couleurs' \n 'balises' \n");
        scanf("%s", fonction);
        // Nettoyage fgets
        int c;
        while ((c = getchar()) != EOF && c != '\n');
        if (strcmp(fonction, "message") == 0)
        {
            printf("Votre message (max 1000 caracteres): ");
            envoie_recois_message(socketfd, data, "message: ");
            done = 1;
        }
        else if (strcmp(fonction, "nom") == 0)
        {
            envoie_nom_de_client(socketfd, data);
            done = 1;
        }
        else if (strcmp(fonction, "calcul") == 0)
        {
            printf("Saisir une operation sous le format : [Operateur / nombre1 / nombre 2]");
            envoie_recois_message(socketfd, data, "calcule: ");
            done = 1;
        }
        else if (strcmp(fonction, "couleurs") == 0)
        {
            printf("Vos couleurs : (Format: nbCouleurs couleur1 couleur2 ... couleurN)\n");
            envoie_recois_message(socketfd, data, "couleurs: ");
            done = 1;
        }
        else if (strcmp(fonction, "balises") == 0)
        {    
            printf("Vos balises : (Format: nbbalises balise1 balise2 ... balisesN)\n");
            envoie_recois_message(socketfd, data, "balises: ");
            done = 1;
        }
        else
        {
            printf("\nFonction inconnue : %s\n", fonction);
        }
    }

    printf("Data : %s\n", data);

    int write_status = write(socketfd, data, strlen(data));
    if (write_status < 0)
    {
        perror("erreur ecriture");
        exit(EXIT_FAILURE);
    }

    // la réinitialisation de l'ensemble des données
    memset(data, 0, sizeof(data));
    // lire les données de la socket
    int read_status = read(socketfd, data, sizeof(data));
    if (read_status < 0)
    {
        perror("erreur lecture");
        return -1;
    }
    printf("Message recu: %s\n", data);
    close(socketfd);
}
