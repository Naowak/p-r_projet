#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <pthread.h>
#include <semaphore.h>

#include "../communication/communication.h"
#include "../utils/utils.h"

#define NUMBER_IMPRESSION 50
#define SIZE_BUFFER_RECEPTION 26
#define SERVEUR_NAME "../serveur/serveur"

int num_connexion;

Impression tab_impression[50];
int indice;
sem_t sem_tab_impression;


void* reception(){
	while(1){
		int result;
		int id, n_commande;
		char* texte;
		result = recevoirOctets(num_connexion, &id, sizeof(int));
		if(result < 0){
			perror("Erreur reception\n");
			exit(1);
		}
		result = recevoirOctets(num_connexion, &n_commande, sizeof(int));
		if(result < 0){
			perror("Erreur reception\n");
			exit(1);
		}
		result = recevoirOctets(num_connexion, &texte, sizeof(char)*SIZE_BUFFER_RECEPTION);
		if(result < 0){
			perror("Erreur reception\n");
			exit(1);
		}
		printf("%d %d %s\n", id, n_commande, texte);
	}
}

int main(int argc, char* argv[]){
	num_connexion = demanderCommunication(SERVEUR_NAME);
	if(num_connexion < 0){
		perror("Erreur Connexion Imprimante\n");
		exit(1);
	}

	indice = 0;
	int result;
	result = sem_init(&sem_tab_impression, 0, 1);
	if(result == -1){
		perror("Erreur Init Semaphore tab impression\n");
		exit(1);
	}

	pthread_t thread_reception;
	pthread_create(&thread_reception, NULL, reception, NULL);
	pthread_join(thread_reception, NULL);
}