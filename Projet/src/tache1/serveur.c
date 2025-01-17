/*
 * SPDX-FileCopyrightText: 2021 John Samuel
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "serveur.h"

void plot(char *data)
{

    //Extraire le compteur et les couleurs RGB
    FILE *p = popen("gnuplot -persist", "w");
    printf("Plot");
    int count = 0;
    int n;
    char *saveptr = NULL;
    char *str = data;
    fprintf(p, "set xrange [-15:15]\n");
    fprintf(p, "set yrange [-15:15]\n");
    fprintf(p, "set style fill transparent solid 0.9 noborder\n");
    fprintf(p, "set title 'Top 10 colors'\n");
    fprintf(p, "plot '-' with circles lc rgbcolor variable\n");
    while (1)
    {
        char *token = strtok_r(str, ",", &saveptr);
        if (token == NULL)
        {
            break;
        }
        str = NULL;
        if (count == 0)
        {
            n = atoi(token);
        }
        else
        {
            // Le numéro 36, parceque 360° (cercle) / 10 couleurs = 36
            fprintf(p, "0 0 10 %d %d 0x%s\n", (count - 1) * 36, count * 36, token + 1);
        }
        count++;
    }
    fprintf(p, "e\n");
    printf("Plot: FIN\n");
    pclose(p);
}

/* renvoyer un message (*data) au client (client_socket_fd)
 */
int renvoie_message(int client_socket_fd, char *data)
{
    int data_size = write(client_socket_fd, (void *)data, strlen(data));

    if (data_size < 0)
    {
        perror("erreur ecriture");
        return (EXIT_FAILURE);
    }
}

// renvoie le nom du client au client
int renvoie_nom(int client_socket_fd, char *data)
{
    return renvoie_message(client_socket_fd, data);
}

// reçoit numero, calcule et renvoie le resultat
int recois_numeros_calcule(int socketfd, char *operator, char * num1, char *num2)
{
    printf("Le calcul à faire est: %s %s %s \n", operator, num1, num2);
    double nb1 = atof(num1);
    double nb2 = atof(num2);
    double resultat;

    char reponse[256];

    if (strcmp(operator, "+") == 0)
    {
        resultat = nb1 + nb2;
    }
    if (strcmp(operator, "-") == 0)
    {
        resultat = nb1 - nb2;
    }
    if (strcmp(operator, "*") == 0)
    {
        resultat = nb1 * nb2;
    }
    if (strcmp(operator, "/") == 0)
    {
        resultat = nb1 / nb2;
    }
    // Conversion float -> chaine de caractère
    sprintf(reponse, "%f", resultat);
    char code[10] = "calcule: ";

    renvoie_message(socketfd, strcat(code, reponse));

    //fermer le socket
    close(socketfd);
}

int recois_couleurs(int socketfd, char args[30][256])
{
    int nb_couleurs = atoi(args[1]);
    printf("%d couleurs à enregistrer\n", nb_couleurs);
    FILE *file = fopen("SauvegardeCouleurs.txt", "w");
    for (int i = 2; i < nb_couleurs + 2; i++)
    {
        printf("%s enregistré !\n", args[i]);
        fputs(args[i], file);
    }

    char message[1024] = "couleurs: Enregistré";
    renvoie_message(socketfd, message);
}

int recois_balises(int socketfd, char args[30][256])
{
    int nb_balises = atoi(args[1]);
    printf("%d balises à enregistrer\n", nb_balises);
    FILE *file = fopen("SauvegardeBalises.txt", "w");
    for (int i = 2; i < nb_balises + 2; i++)
    {
        printf("%s enregistré !\n", args[i]);
        fputs(args[i], file);
    }

    char message[1024] = "balises: Enregistré";
    renvoie_message(socketfd, message);
}

/* accepter la nouvelle connexion d'un client et lire les données
 * envoyées par le client. Ensuite, le serveur envoie un message
 * en retour
 */
int recois_envoie_message(int socketfd)
{
    struct sockaddr_in client_addr;
    char data[1024];

    int client_addr_len = sizeof(client_addr);

    // nouvelle connection de client
    int client_socket_fd = accept(socketfd, (struct sockaddr *)&client_addr, &client_addr_len);
    if (client_socket_fd < 0)
    {
        perror("accept");
        return (EXIT_FAILURE);
    }

    // la réinitialisation de l'ensemble des données
    memset(data, 0, sizeof(data));

    //lecture de données envoyées par un client
    int data_size = read(client_socket_fd, (void *)data, sizeof(data));

    if (data_size < 0)
    {
        perror("erreur lecture");
        return (EXIT_FAILURE);
    }

    /*
   * extraire le code des données envoyées par le client. 
   * Les données envoyées par le client peuvent commencer par le mot "message :" ou un autre mot.
   */
    printf("Message recu: %s\n", data);
    char code[10];
    char args[30][256];
    char message[256];
    int n;
    int i = 0;
    char *input = data;
    printf("%s", data);
    while (sscanf(input, " %s%n", args[i], &n) == 1)
    {
        printf("\n[%s]\n", args[i]);
        i++;
        input += n;
    }
    strcpy(code, args[0]);
    char charMsg[20] = "message: ";

    //Si le message commence par le mot: 'message:'
    if (strcmp(code, "message:") == 0)
    {
        printf("Saisir message: ");
        scanf("%s", message);
        renvoie_message(client_socket_fd, strcat(charMsg, message));
    }
    else if (strcmp(code, "nom:") == 0)
    {
        renvoie_message(client_socket_fd, data);
    }
    else if (strcmp(code, "calcule:") == 0)
    {
        recois_numeros_calcule(client_socket_fd, args[1], args[2], args[3]);
    }
    else if (strcmp(code, "couleurs:") == 0)
    {
        recois_couleurs(client_socket_fd, args);
    }
    else if (strcmp(code, "balises:") == 0)
    {
        recois_balises(client_socket_fd, args);
    }
    else
    {
        plot(data);
    }

    //fermer le socket
    close(socketfd);
}

int main()
{

    int socketfd;
    int bind_status;
    int client_addr_len;

    struct sockaddr_in server_addr, client_addr;

    /*
   * Creation d'une socket
   */
    socketfd = socket(AF_INET, SOCK_STREAM, 0);
    if (socketfd < 0)
    {
        perror("Unable to open a socket");
        return -1;
    }

    int option = 1;
    setsockopt(socketfd, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option));

    //détails du serveur (adresse et port)
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    // Relier l'adresse à la socket
    bind_status = bind(socketfd, (struct sockaddr *)&server_addr, sizeof(server_addr));
    if (bind_status < 0)
    {
        perror("bind");
        return (EXIT_FAILURE);
    }

    // Écouter les messages envoyés par le client
    listen(socketfd, 10);

    //Lire et répondre au client
    recois_envoie_message(socketfd);

    return 0;
}