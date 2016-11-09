#ifndef CLIENT_H
#define CLIENT_H

#include "../communication/communication.h"
#include "../utils/utils.h"

typedef unsigned int ID_IMPRESSION;
/*
demande la connection au serveur passe en parametre
retourne si la connexion est reussit
 */
bool connect(string serveur_name);
/*

 */
bool stop( ID_IMPRESSION num_file);
/*

 */
bool
#endif
