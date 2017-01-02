#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <pthread.h>

#include "../communication/communication.h"
#include "../utils/utils.h"

#define SIZE_BUFFER 30
#define MAX_NUMBER_INSTRUCTONS 30

int pipe_envois[2];
int pipe_retour[2];


void* gestion_instruction(){
	char buffer[SIZE_BUFFER];
	int n_instruction = 0;
	while(1){
		read(STDIN_FILENO, buffer, SIZE_BUFFER);
		Instruction i = {n_instruction++, atoi(&buffer[0]), atoi(&buffer[2]), buffer + 4};
		printf("gi : %d %d %d %s\n", i.id, i.n_printer, i.n_commande, i.texte);
		memset(buffer, 0, SIZE_BUFFER);
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

	pthread_t thread_gestion_instruction;
	pthread_create(&thread_gestion_instruction, NULL, gestion_instruction, NULL);
	pthread_join(thread_gestion_instruction, NULL);


	return 0;
}