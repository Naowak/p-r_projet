/*
 ============================================================================
 Name        : client.c
 Author      : Amal Sayah
 ============================================================================
 */

#include "client.h"

bool connexion(const char *serveur, const char *fichierDemande) {
  int numCommunication;
  int nbRecus;
  char tampon[BUFSIZ];
  int lgChemin;
  int nbEnvoyes;

  // se connecter au serveur
  if ((numCommunication = demanderCommunication(serveur)) < 0) {
    fprintf(stderr, "Client: erreur %s\n",
            messageErreur((RetourCommunication)numCommunication));
    return 1;
  }

  // recevoir ensuite le contenu du fichier s'il existe
  while ((nbRecus = recevoirOctets(numCommunication, tampon, BUFSIZ)) > 0)
    write(1, tampon, nbRecus);

  // clore la communication
  cloreCommunication(numCommunication);

  return 0;
}

int main(int argc, char *argv[]) {
  int ret;
  if (argc != 2) {
    fprintf(stderr, "usage: %s nom_serveur ", argv[0]);
    exit(2);
  }
  ret = connexion(argv[1]);
  exit(ret);
}
