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
#include <regex.h>

#include "client.h"
#include "bmp.h"

// Rien d'écrit en dur pour plus de flexibilité
#define REG_MSG "{[[:space:]]*\"[[:alpha:]]*\"[[:space:]]*:[[:space:]]*\"[[:alpha:]]*\",[[:space:]]*\"[[:alpha:]]*\"[[:space:]]*:[[:space:]]*\\[[[:space:]]*\"[[:graph:]]*\"[[:space:]]*\\][[:space:]]*}"

// Retourne 0 si JSON valide, 1 sinon.
int verifieValiditeJSON(char *JSON) {
    regex_t regex;
    regcomp(&regex, REG_MSG, 0);
    return regexec(&regex, JSON, 0, NULL, 0);
}