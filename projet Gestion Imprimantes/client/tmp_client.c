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

Impression tab_impression[NUMBER_IMPRESSION];
int indice;
sem_t sem_tab_impression[NUMBER_IMPRESSION];

int pipe_com_rec_imp_rec[2];


void* reception(){
	while(1){
		int result;
		int id, n_commande;
		char texte[30];
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
		result = recevoirOctets(num_connexion, texte, sizeof(char)*SIZE_BUFFER_RECEPTION);
		if(result < 0){
			perror("Erreur reception\n");
			exit(1);
		}
		//printf("rec : %d %d %s\n", id, n_commande, texte);

		switch(n_commande){
			case IMPRIMER :
			{
				printf("%s\n", texte);
				write(pipe_com_rec_imp_rec[1], &id, sizeof(int));
				write(pipe_com_rec_imp_rec[1], texte, sizeof(char)*strlen(i.texte));
				break;
			}
			case ANNULER :
				break;
			case ETAT_IMPRIMANTE :
				break;
			case ETAT_IMPRESSION :
				break;
			default :
				perror("Erreur commande inconnu\n");
				break;
		}
	}
}

void* imprimer_reception(){
	while(1){
		Impression i;
		while(read(pipe_com_rec_imp_rec[0], &i.id, sizeof(int)) == 0);
		while(read(pipe_com_rec_imp_rec[0], i.texte, sizeof(char)*SIZE_BUFFER_RECEPTION) == 0);
		sem_wait(sem_tab_impression[indice]);
		tab_impression[indice] = i;
		sem_post(sem_tab_impression[indice++]);
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

	pipe(pipe_com_rec_imp_rec);	

	pthread_t thread_reception, thread_imprimer_reception;
	pthread_create(&thread_reception, NULL, reception, NULL);
	pthread_create(&thread_imprimer_reception, NULL, imprimer_reception, NULL);
	pthread_join(thread_reception, NULL);
	pthread_join(thread_imprimer_reception, NULL);
}