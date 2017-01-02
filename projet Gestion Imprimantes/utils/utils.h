#ifndef UTILS_H
#define UTILS_H

#include "../communication/communication.h"

#define SERVEUR_NAME "serveur"

typedef enum {IMPRIMER, ANNULER, ETAT_IMPRIMANTE, ETAT_IMPRESSION} commande;

typedef struct instruction_t Instruction;
struct instruction_t{
	int id;
	int n_printer;
	commande n_commande;
	char* texte;
};

#endif
