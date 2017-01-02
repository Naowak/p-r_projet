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

int pipe_envois[2];
int pipe_retour[2];

Instruction tab_inst[MAX_NUMBER_INSTRUCTONS];
int tab_imprimantes[NUMBER_IMPRIMANTES];

int num_serveur;


void* gestion_instruction(){
	close(pipe_envois[0]);
	close(pipe_retour[0]);
	close(pipe_retour[1]);
	char buffer[SIZE_BUFFER];
	int n_instruction = 0;
	while(1){
		read(STDIN_FILENO, buffer, SIZE_BUFFER);
		Instruction i = {n_instruction, atoi(&buffer[0]), atoi(&buffer[2]), buffer + 4};
		tab_inst[n_instruction++] = i;
		//printf("gi : %d %d %d %s\n", i.id, i.n_printer, i.n_commande, i.texte);
		write(pipe_envois[1], &i.id, sizeof(int));
		write(pipe_envois[1], &i.n_printer, sizeof(int));
		write(pipe_envois[1], &i.n_commande, sizeof(int));
		write(pipe_envois[1], &i.texte, sizeof(char)*SIZE_BUFFER);
		memset(buffer, 0, SIZE_BUFFER);
	}
}



void* connexion(){
	close(pipe_envois[0]);
	close(pipe_envois[1]);
	close(pipe_retour[0]);
	close(pipe_retour[1]);
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



	pthread_t thread_gestion_instruction, thread_connexion;
	pthread_create(&thread_gestion_instruction, NULL, gestion_instruction, NULL);
	pthread_create(&thread_connexion, NULL, connexion, NULL);
	pthread_join(thread_gestion_instruction, NULL);
	pthread_join(thread_connexion, NULL);


	return 0;
}