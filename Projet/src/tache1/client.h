/*
 * SPDX-FileCopyrightText: 2021 John Samuel
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#ifndef __CLIENT_H__
#define __CLIENT_H__

/*
 * port d'ordinateur pour envoyer et recevoir des messages
 */
#define PORT 8089

/* 
 * Fonction d'envoi et de réception de messages
 * Il faut des arguments : l'identifiant de la socket, la chaîne de caractère à modifier, le code de la fonction utilisée.
 */
int envoie_recois_message(int socketfd, char* data, char* code);

#endif
