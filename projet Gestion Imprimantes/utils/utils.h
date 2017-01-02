#ifndef UTILS_H
#define UTILS_H

#include "../communication/communication.h"

typedef enum {IMPRIMER=0, ANNULER=1, ETAT_IMPRIMANTE=2, ETAT_IMPRESSION=3} commande;

typedef struct instruction_t Instruction;
struct instruction_t{
	int id;
	int n_printer;
	commande n_commande;
	char* texte;
};

typedef struct impression_t Impression;
struct impression_t{
	int id;
	char* texte;
};

#endif
