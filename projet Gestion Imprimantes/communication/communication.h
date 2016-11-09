#ifndef COMMUNICATION_H
#define COMMUNICATION_H

typedef enum {OK=0, ERR_SOCKET_UNIX=-1, ERR_NOM_IMPRIMANTE=-2, ERR_LISTEN_UNIX=-3, ERR_ACCEPT_UNIX=-4,
			  ERR_READ_UNIX=-5, ERR_WRITE_UNIX=-6, ERR_CLOSE_UNIX=-7, ERR_GETSOCKNAME_UNIX=-8,
			  ERR_UNLINK_UNIX=-9, ERR_CONNECT_UNIX=-10, LAST=-11} RetourCommunication;


// Obtenir le message texte explicitant un code de retour RetourCommunication
const char *messageErreur(RetourCommunication ret);

// initialiser dans un processsu le système de communication pour qu'il puisse établir des communications
// avec des processus clients
// A appeler une seule fois, sinon retour d'erreur
// en entrée: le nom unique du processus "serveur de communication"
// en retour: >= 0: le numéro du serveur de la communication distante à utiliser par la suite
//            < 0 : code d'erreur
int initialiserServeur(const char *nomServeur);

// le serveur accepte une nouvelle communication avec un processus client
// en entrée: le numéro du serveur de communication concernée(obtenu par initialiserServeur)
// en retour: >= 0 : le numéro de la communication initialisée, à utiliser pour les échanges de données ultérieurs
//            <  0 : code d'erreur
int accepterCommunication(int numServeur);

// Un client demande l'initialisation d'une communication avec le serveur
// en retour: >= 0 : le numéro de la communication initialisée, à utiliser pour les échanges de données ultérieurs
//            <  0 : code d'erreur
int demanderCommunication(const char *nomServeur);

// envoyer ou recevoir des flots d'octets dans le cadre d'une communication
// en entrée: le numéro de la communication concernée, l'adresse de la zone mémoire où lire ou où écrire
//            et le nombre d'octets à lire ou à écrire
// en retour: >= 0 : le nombre d'octets effectivement lus
//            < 0  : code d'ereur
int recevoirOctets(int numCommunication, void *destination, unsigned nbMaxOctets);
int envoyerOctets(int numCommunication, const void *source, unsigned nbMaxOctets);

// Fin d'une communication, plus aucun nouvel échange ne peut être effectué (envoyer/recevoir)
// en entrée: le numéro de la communication concernée
// en retour: 0 :  OK
//            <  0 : code d'erreur
int cloreCommunication(int numCommunication);

// Arrêt du service, plus aucune nouvelle communication ne peut être initialisée
// en entrée: le numéro du serveur de communication
// en retour: 0 :  OK
//           < 0 : code d'erreur
int arreterServeur(int numServeur);

#endif
