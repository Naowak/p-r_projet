#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <pthread.h>

#include "../communication/communication.h"
#include "../utils/utils.h"

#define SIZE_BUFFER 30
#define MAX_NUMBER_INSTRUCTONS 100
#define NUMBER_IMPRIMANTES 20
#define SERVEUR_NAME "serveur"

int pipe_envois[2];
int pipe_retour[2];

Instruction tab_inst[MAX_NUMBER_INSTRUCTONS];
int tab_imprimantes[NUMBER_IMPRIMANTES];

int num_serveur;


void* gestion_instruction(){
	char buffer[SIZE_BUFFER];
	int n_instruction = 0;
	while(1){
		//while(read(STDIN_FILENO, buffer, SIZE_BUFFER) <= 0);
		Instruction i ;//= {n_instruction, atoi(&buffer[0]), atoi(&buffer[2]), buffer + 4};
		int tmp_n_commande;
		i.texte = malloc(sizeof(char)*SIZE_BUFFER);
		scanf("%d %d %s",&i.n_printer,&tmp_n_commande, i.texte);
		i.n_commande = (commande) tmp_n_commande;
		i.id = n_instruction;
		tab_inst[n_instruction++] = i;
		//printf("gi : %d %d %d %s\n", i.id, i.n_printer, i.n_commande, i.texte);
		write(pipe_envois[1], &i.id, sizeof(int));
		write(pipe_envois[1], &i.n_printer, sizeof(int));
		write(pipe_envois[1], &i.n_commande, sizeof(int));
		write(pipe_envois[1], i.texte, sizeof(char)*strlen(i.texte));
		free(i.texte);
		memset(buffer, 0, SIZE_BUFFER);
	}
}

void* communication_envois(){
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



void* connexion(){
	int result;
	int cmp = 0;
	while(1){
		result = accepterCommunication(num_serveur);
		if(result > 0){
			printf("Nouvelle Imprimante Connectée au Serveur : %d\n", result);
			tab_imprimantes[cmp++] = result;
		}
	}
}



int main(int argc, char* argv[]){
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



	pthread_t thread_gestion_instruction, thread_com_envois, thread_connexion;
	pthread_create(&thread_gestion_instruction, NULL, gestion_instruction, NULL);
	pthread_create(&thread_com_envois, NULL, communication_envois, NULL);
	pthread_create(&thread_connexion, NULL, connexion, NULL);
	pthread_join(thread_gestion_instruction, NULL);
	pthread_join(thread_com_envois, NULL);
	pthread_join(thread_connexion, NULL);


	return 0;
}