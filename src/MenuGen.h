#ifndef MenuGen_h
#define MenuGen_h

// Tp3  manipulation MenuGen avec PEC12
// C. HUBER  03.02.2016
// Fichier MenuGen.h
// Gestion du menu  du générateur
// Traitement cyclique à 1 ms du Pec12


#include <stdbool.h>
#include <stdint.h>
#include "DefMenuGen.h"

#define MENU_SELECT  0
#define MENU_EDIT    1
#define MENU_SAVE    2
#define MENU_WAIT_SAVE    3

#define SAVE_DATA   0
#define UNSAVE_DATA 1

#define FORME   0
#define FREQUENCE   1
#define AMPLITUDE   2
#define OFFSET  3

#define PUSH_WAIT_SAVE  50

#define MAX_AMPLITUDE   10000
#define MIN_AMPLITUDE   0
#define PAS_AMPLITUDE   100

#define MAX_FREQUENCE   2000
#define MIN_FREQUENCE   20
#define PAS_FREQUENCE   20

#define MAX_OFFSET  5000
#define MIN_OFFSET  -5000
#define PAS_OFFSET  100

#define TIME_WAIT_SAVE_MS 200

void MENU_Initialize(S_ParamGen *pParam);

void MENU_Execute(S_ParamGen *pParam,bool local);

void MENU_DemandeSave(void);

#endif