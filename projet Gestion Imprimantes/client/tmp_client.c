#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdbool.h>

#include "../communication/communication.h"
#include "../utils/utils.h"

#define NUMBER_IMPRESSION 50
#define SIZE_BUFFER_RECEPTION 30
#define SERVEUR_NAME "../serveur/serveur"

int num_connexion;

Impression tab_impression[NUMBER_IMPRESSION];
sem_t sem_tab_impression[NUMBER_IMPRESSION];
sem_t places_tab_impression;
bool est_disponible_tab_impression[NUMBER_IMPRESSION];

int pipe_com_rec_imp_rec[2];
int pipe_imp_rec_imp_env[2];

int pipe_retour[2];
sem_t sem_pipe_retour;

char imprimante_name[15];

bool est_distante = false;


void* reception(){
	/** Receptionne les messages reçus par le serveur **/
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

		switch(n_commande){
			case IMPRIMER :
			{
				write(pipe_com_rec_imp_rec[1], &id, sizeof(int));
				write(pipe_com_rec_imp_rec[1], texte, sizeof(char)*strlen(texte));
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

void* envoyer(){
	/** Envois les messages de retour d'instruction au serveur **/
	int id;
	int retour;
	int result;
	while(1){
		while(read(pipe_retour[0], &id, sizeof(int)) <= 0);
		while(read(pipe_retour[0], &retour, sizeof(int)) <= 0);

		result = envoyerOctets(num_connexion, &id, sizeof(int));
		if(result < 0){
			perror("Erreur Envois Octets");
			exit(1);
		}
		result = envoyerOctets(num_connexion, &retour, sizeof(int));
		if(result < 0){
			perror("Erreur Envois Octets");
			exit(1);
		}
	}
}

void* imprimer_reception(){
	/** Ajoute à la liste d'impression les impressions demandées par le serveur **/
	while(1){
		Impression i;
		i.texte = malloc(sizeof(char)*SIZE_BUFFER_RECEPTION);
		while(read(pipe_com_rec_imp_rec[0], &i.id, sizeof(int)) <= 0);
		while(read(pipe_com_rec_imp_rec[0], i.texte, sizeof(char)*SIZE_BUFFER_RECEPTION) <= 0);

		sem_wait(&places_tab_impression);
		int indice;
		for(indice = 0; indice < NUMBER_IMPRESSION; ++indice)
			if(est_disponible_tab_impression[indice]){
				sem_wait(&sem_tab_impression[indice]);
				est_disponible_tab_impression[indice] = false;
				tab_impression[indice] = i;
				sem_post(&sem_tab_impression[indice]);
				break;
			}

		write(pipe_imp_rec_imp_env[1], &indice, sizeof(int));
	}
}

void* imprimer_envois(){
	/** Imprime et donne son retour (réussi ou non) à la fonction d'envois **/
	while(1){
		int indice, retour;
		while(read(pipe_imp_rec_imp_env[0], &indice, sizeof(int)) <= 0);

		sem_wait(&sem_tab_impression[indice]);

		if(!est_disponible_tab_impression[indice]){
			int file_lecture = open(tab_impression[indice].texte, O_RDONLY);
			if(file_lecture == -1){
				perror("Erreur ouverture file_lecture 1\n");
				exit(1);
			}
			int file_ecriture;
			if(est_distante){
				file_ecriture = open(imprimante_name, O_WRONLY | O_APPEND | O_CREAT, 0666);
				if(file_lecture == -1){
					perror("Erreur ouverture file_ecriture 2\n");
					exit(1);
				}
			}
			else
				file_ecriture = STDOUT_FILENO;

			char c;
			int result;
			do{
				result = read(file_lecture, &c, sizeof(char));
				if(result > 0)
					write(file_ecriture, &c, sizeof(char));
			}while(result > 0);

			est_disponible_tab_impression[indice] = true;
			printf("Impression du fichier %s\n.", tab_impression[indice].texte);

			close(file_ecriture);
			close(file_lecture);

			retour = 1;
		}
		else{
			retour = 0;
		}
		sem_wait(&sem_pipe_retour);
		write(pipe_retour[1], &tab_impression[indice].id, sizeof(int));
		write(pipe_retour[1], &retour, sizeof(int));
		sem_post(&sem_pipe_retour);
		sem_post(&sem_tab_impression[indice]);
		sem_post(&places_tab_impression);
	}
}

int main(int argc, char* argv[]){
	/** Initialise les variables et lance les threads **/
	if(argc != 2 && argc != 3){
		printf("Usage : <nom_imprimante> <optionnel : 1 si distante>\n");
		exit(1);
	}
	char* nom_imprimante = argv[1];
	if(argc == 3)
		est_distante = true;

	num_connexion = demanderCommunication(SERVEUR_NAME);
	if(num_connexion < 0){
		perror("Erreur Connexion Imprimante\n");
		exit(1);
	}
	else{
		if(est_distante)
			printf("Imprimante Distante %s Connectée.\n", nom_imprimante);
		else
			printf("Imprimante %s Connectée.\n", nom_imprimante);
	}

	int result;
	int i;
	for(i = 0; i < NUMBER_IMPRESSION; ++i){
		result = sem_init(&sem_tab_impression[i], 0, 1);
		if(result == -1){
			perror("Erreur Init Semaphore tab impression\n");
			exit(1);
		}
	}
	result = sem_init(&places_tab_impression, 0, NUMBER_IMPRESSION);
	if(result == -1){
		perror("Erreur Init Semaphore places_tab_impression\n");
		exit(1);
	}
	result = sem_init(&sem_pipe_retour, 0, 1);
	if(result == -1){
		perror("Erreur Init Semaphore sem_pipe_retour\n");
		exit(1);
	}

	pipe(pipe_com_rec_imp_rec);
	pipe(pipe_imp_rec_imp_env);
	pipe(pipe_retour);

	for(i = 0; i < NUMBER_IMPRESSION; ++i)
		est_disponible_tab_impression[i] = true;

	sprintf(imprimante_name, "imprimante_%s", nom_imprimante);

	pthread_t thread_reception, thread_imprimer_reception, thread_imprimer_envois;
	pthread_t thread_envoyer;
	pthread_create(&thread_reception, NULL, reception, NULL);
	pthread_create(&thread_imprimer_reception, NULL, imprimer_reception, NULL);
	pthread_create(&thread_imprimer_envois, NULL, imprimer_envois, NULL);
	pthread_create(&thread_envoyer, NULL, envoyer, NULL);
	pthread_join(thread_reception, NULL);
	pthread_join(thread_imprimer_reception, NULL);
	pthread_join(thread_imprimer_envois, NULL);
	pthread_join(thread_envoyer, NULL);

	return 0;
}