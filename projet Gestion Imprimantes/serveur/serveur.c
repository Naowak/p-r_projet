#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <pthread.h>
#include <semaphore.h>

#include "../communication/communication.h"
#include "../utils/utils.h"

#define SIZE_BUFFER 30
#define MAX_NUMBER_INSTRUCTONS 100
#define NUMBER_IMPRIMANTES 20
#define SERVEUR_NAME "serveur"

int pipe_envois[2];
int pipe_retour[2];
sem_t sem_pipe_retour;

Instruction tab_inst[MAX_NUMBER_INSTRUCTONS];
int tab_imprimantes[NUMBER_IMPRIMANTES];

int num_serveur;


void* gestion_instruction(){
	/** Lit l'entrée standard et redirige l'instruction vers la fonction d'envois **/
	char buffer[SIZE_BUFFER];
	int n_instruction = 0;
	while(1){
		Instruction i ;
		int tmp_n_commande;
		i.texte = malloc(sizeof(char)*SIZE_BUFFER);
		scanf("%d %d %s",&i.n_printer,&tmp_n_commande, i.texte);
		i.n_commande = (commande) tmp_n_commande;
		i.id = n_instruction;
		tab_inst[n_instruction++] = i;
		
		write(pipe_envois[1], &i.id, sizeof(int));
		write(pipe_envois[1], &i.n_printer, sizeof(int));
		write(pipe_envois[1], &i.n_commande, sizeof(int));
		write(pipe_envois[1], i.texte, sizeof(char)*strlen(i.texte));
		free(i.texte);
		memset(buffer, 0, SIZE_BUFFER);
	}
}

void* communication_envois(){
	/** Envois les instructions aux imprimantes correspondantes **/
	int result;
	Instruction i;
	i.texte = malloc(sizeof(char)*SIZE_BUFFER);
	while(1){
		while(read(pipe_envois[0], &(i.id), sizeof(int)) <= 0 );
		while(read(pipe_envois[0], &i.n_printer, sizeof(int)) <= 0);
		while(read(pipe_envois[0], &i.n_commande, sizeof(int)) <= 0);
		while(read(pipe_envois[0], i.texte, sizeof(char)*SIZE_BUFFER) <= 0);

		result = envoyerOctets(i.n_printer, &i.id, sizeof(int));
		if(result < 0){
			perror("Erreur Envois Octets\n");
			exit(1);
		}
		result = envoyerOctets(i.n_printer, &i.n_commande, sizeof(int));
		if(result < 0){
			perror("Erreur Envois Octets\n");
			exit(1);
		}
		result = envoyerOctets(i.n_printer, i.texte, sizeof(char)*(SIZE_BUFFER - 4));
		if(result < 0){
			perror("Erreur Envois Octets\n");
			exit(1);
		}

		memset(i.texte, 0, sizeof(char)*SIZE_BUFFER);
		printf("La requête %d a été envoyée.\n", i.id);
	}
	free(i.texte);
}


void* communication_reception_retour(void* numero_imprimante){
	/** Receptionne les retours d'instructions des imprimantes. 
	Il existe un thread tournant sur cette fonction par imprimante **/
	int nb_imp = *((int *) numero_imprimante);
	int id, retour;
	while(1){
		int result;
		result = recevoirOctets(nb_imp, &id, sizeof(int));
		if(result < 0){
			perror("Erreur reception\n");
			exit(1);
		}
		result = recevoirOctets(nb_imp, &retour, sizeof(int));
		if(result < 0){
			perror("Erreur reception\n");
			exit(1);
		}

		sem_wait(&sem_pipe_retour);
		write(pipe_retour[1], &id, sizeof(int));
		write(pipe_retour[1], &retour, sizeof(int));
		sem_post(&sem_pipe_retour);
	}
}

void* connexion(){
	/** Gère les demandes de connexions de la part des imprimantes **/
	int result;
	int cmp = 0;
	pthread_t tab_thread_retour[NUMBER_IMPRIMANTES];
	while(1){
		result = accepterCommunication(num_serveur);
		if(result > 0){
			printf("Nouvelle Imprimante Connectée au Serveur : %d\n", result);
			tab_imprimantes[cmp] = result;
			pthread_create(&tab_thread_retour[cmp++], NULL, communication_reception_retour, &result);
		}
	}
}


void* gestion_retour(){
	/** Gère le retour des instructions données aux imprimantes **/
	int id;
	int retour;
	while(1){
		while(read(pipe_retour[0], &id, sizeof(int)) <= 0);
		while(read(pipe_retour[0], &retour, sizeof(int)) <= 0);

		switch(tab_inst[id].n_commande){
			case IMPRIMER :
				if(retour)
					printf("L'impression ayant pour requête %d s'est terminée avec succès.\n", id);
				else
					printf("L'impression ayant pour requête %d n'existe plus.\n", id);
				break;
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



int main(int argc, char* argv[]){
	/** Initialise les variables et lancer les threads **/
	int result;
	result = pipe(pipe_envois);
	if(result < 0 ){
		perror("Error Pipe 1\n");
		exit(1);
	}
	result = pipe(pipe_retour);
	if(result < 0 ){
		perror("Error Pipe 1\n");
		exit(1);
	}

	unlink(SERVEUR_NAME);
	num_serveur = initialiserServeur(SERVEUR_NAME);
	if(num_serveur < 0){
		perror("Error Init Server\n");
		exit(1);
	}

	int i;
	for(i = 0; i < NUMBER_IMPRIMANTES; ++i)
		tab_imprimantes[i] = -1;

	result = sem_init(&sem_pipe_retour, 0, 1);
	if(result == -1){
		perror("Erreur init sem_pipe_retour\n");
		exit(1);
	}


	pthread_t thread_gestion_instruction, thread_com_envois, thread_connexion;
	pthread_t thread_gestion_retour;
	pthread_create(&thread_gestion_instruction, NULL, gestion_instruction, NULL);
	pthread_create(&thread_com_envois, NULL, communication_envois, NULL);
	pthread_create(&thread_connexion, NULL, connexion, NULL);
	pthread_create(&thread_gestion_retour, NULL, gestion_retour, NULL);
	pthread_join(thread_gestion_instruction, NULL);
	pthread_join(thread_com_envois, NULL);
	pthread_join(thread_connexion, NULL);
	pthread_join(thread_gestion_retour, NULL);


	return 0;
}