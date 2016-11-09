
/*
 * communication.c
 *
 *  Created on: Jun 22, 2016
 *      Author: Amal Sayah
 */

#include "communication.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <fcntl.h>


static const char *messagesErreur[] =
	{"OK", "ERR_SOCKET_UNIX", "ERR_NOM_IMPRIMANTE", "ERR_LISTEN_UNIX", "ERR_ACCEPT_UNIX",
	 "ERR_READ_UNIX", "ERR_WRITE_UNIX", "ERR_CLOSE_UNIX", "ERR_GETSOCKNAME_UNIX",
	 "ERR_UNLINK_UNIX", "ERR_CONNECT_UNIX"};

// Obtenir le message texte explicitant un code de retour RetourCommunication
const char *messageErreur(RetourCommunication ret)

{
	if (ret <= LAST || ret > OK)
		return "Erreur inconnue";
	return messagesErreur[-ret];
}


// initialiser dans un processsu le système de communication pour qu'il puisse établir des communications
// avec des processus clients
// A appeler une seule fois, sinon retour d'erreur
// en entrée: le nom unique du processus "serveur de communication"
// en retour: >= 0: le numéro du serveur de la communication distante à utiliser par la suite
//            < 0 : code d'erreur
int initialiserServeur(const char *nomServeur)
{
	int numServeur;
	struct sockaddr_un infoBind;
	unsigned lgInfo;

	if ((numServeur = socket(AF_UNIX, SOCK_STREAM, 0)) == -1)
	return ERR_SOCKET_UNIX;

	lgInfo = strlen(nomServeur);
	strcpy(infoBind.sun_path, nomServeur);
	infoBind.sun_family = AF_UNIX;
	if (bind(numServeur, (struct sockaddr *)&infoBind, lgInfo + sizeof(short)) == -1)
	return ERR_NOM_IMPRIMANTE;

	if (listen(numServeur, 10) == -1)
	 return ERR_LISTEN_UNIX;
	return numServeur;
}


// le serveur accepte une nouvelle communication avec un processus client
// en entrée: le numéro du serveur de communication concernée(obtenu par initialiserServeur)
// en retour: >= 0 : le numéro de la communication initialisée, à utiliser pour les échanges de données ultérieurs
//            <  0 : code d'erreur
int accepterCommunication(int numServeur)
{
	struct sockaddr infoClient;
	int lgInfo = sizeof(infoClient);
	int numCommunication;
    if ((numCommunication = accept(numServeur, (struct sockaddr *)&infoClient, (socklen_t *)&lgInfo)) == -1)
		 return ERR_ACCEPT_UNIX;
	return numCommunication;
}


// Un client demande l'initialisation d'une communication avec le serveur
// en retour: >= 0 : le numéro de la communication initialisée, à utiliser pour les échanges de données ultérieurs
//            <  0 : code d'erreur
int demanderCommunication(const char *nomServeur)
{
	int numCommunication;
	int lgInfo;
	struct sockaddr_un infoConnect;

	// etablir la connexion avec le serveur
	if ((numCommunication = socket(AF_UNIX, SOCK_STREAM, 0)) == -1)
	   return ERR_SOCKET_UNIX;

	lgInfo = strlen(nomServeur);
	strcpy(infoConnect.sun_path, nomServeur);
	infoConnect.sun_family = AF_UNIX;
	if (connect(numCommunication, (struct sockaddr *)&infoConnect,  lgInfo + sizeof(short)) == -1)
		return ERR_CONNECT_UNIX;
	return numCommunication;
}

// envoyer ou recevoir  des flots d'octets dans le cadre d'une communication
// en entrée: le numéro de la communication concernée, l'adresse de la zone mémoire où lire ou où écrire
//            et le nombre d'octets à lire ou à écrire
// en retour: >= 0 : le nombre d'octets effectivement lus
//            < 0  : code d'ereur
int recevoirOctets(int numCommunication, void *destination, unsigned nbMaxOctets)
{
	int nbLus;
	if ((nbLus = read(numCommunication, destination, nbMaxOctets)) > 0)
		return nbLus;
	return ERR_READ_UNIX;
}

int envoyerOctets(int numCommunication, const void *source, unsigned nbMaxOctets)
{
	int nbEcrits;
	if ((nbEcrits = write(numCommunication, source, nbMaxOctets)) > 0)
		return nbEcrits;
	return ERR_WRITE_UNIX;
}

// Fin d'une communication, plus aucun nouvel échange ne peut être effectué (envoyer/recevoir)
// en entrée: le numéro de la communication concernée
// en retour: 0 :  OK
//            <  0 : code d'erreur
int cloreCommunication(int numCommunication)
{
	return ((close(numCommunication) == 0) ? OK : ERR_CLOSE_UNIX);
}

// Arrêt du service, plus aucune nouvelle communication ne peut être initialisée
// en entrée: le numéro du serveur de communication
// en retour: 0 :  OK
//           < 0 : code d'erreur
int arreterServeur(int numServeur)
{
	struct sockaddr_un infoServeur;
	socklen_t lgInfo = sizeof(infoServeur);
	if (getsockname(numServeur, (struct sockaddr *)&infoServeur, (socklen_t *)&lgInfo) == -1)
		 return ERR_GETSOCKNAME_UNIX;
	if (close(numServeur) != 0)
		return ERR_CLOSE_UNIX;
	if (unlink(infoServeur.sun_path) != 0)
		return ERR_UNLINK_UNIX;
	return OK;

}
