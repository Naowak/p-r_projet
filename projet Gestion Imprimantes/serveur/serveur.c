/*
 ============================================================================
 Name        : client.c
 Author      : Amal Sayah
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

#include "../communication/communication.h"

void serveur(const char *serveur)
{
  int numServeur, numCommunication;
  char tampon[BUFSIZ];
  int nbRecus;

  unlink(serveur);

  // initialiser le serveur pour qu'il recoive les requetes
   if ((numServeur = initialiserServeur(serveur)) < 0)
  	  {
	  fprintf(stderr, "Erreur initialiserServeur: %s: %s\n",
			  serveur, messageErreur(numServeur));
	  exit(1);
  	  }
  for (;;)
	 {
	 // accepter une connection
	 if ((numCommunication = accepterCommunication(numServeur)) < 0)
		 {
		 fprintf(stderr, "Erreur accepterCommunication: %s: %s\n",
				 serveur, messageErreur(numCommunication));
	 	 exit(1);
	   	 }

	 // lire le nom du fichier à transferer
	 if ((nbRecus = recevoirOctets(numCommunication, tampon, BUFSIZ)) > 0)
		 {
		 int inputFile;
		 if ((inputFile = open(tampon, O_RDONLY)) != -1)
			{
			// envoyer le cntenu du fichier demande
			int nbLus;
		 	while ((nbLus = read(inputFile, tampon, BUFSIZ)) > 0)
		 		{
		 		if (envoyerOctets(numCommunication, tampon, nbLus) != nbLus)
		 			break;
		 		}
		 	close(inputFile);
			}
		 }

	 // clore la communication
	 cloreCommunication(numCommunication);
	 }
}

int main(int argc, char *argv[])
{
  int ret;
  if (argc != 2)
  	  {
	  fprintf(stderr, "usage: %s nom_serveur\n", argv[0]);
	  exit(2);
  	  }
  serveur(argv[1]);
  exit(ret);
}
