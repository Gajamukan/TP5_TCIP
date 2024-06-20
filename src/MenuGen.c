// Tp3  manipulation MenuGen avec PEC12
// C. HUBER  10/02/2015 pour SLO2 2014-2015
// Fichier MenuGen.c
// Gestion du menu  du générateur
// Traitement cyclique à 10 ms


#include "app.h"
#include <stdint.h>                   
#include <stdbool.h>
#include "MenuGen.h"
#include "Mc32DriverLcd.h"
#include "GesPec12.h"
#include "Mc32Debounce.h"
#include "Generateur.h"
#include "Mc32gestI2cSeeprom.h"

const char MenuFormes[4][21] = { " Sinus   ", " Triangle   ", " DentDeScie", " Carre     " };

S_SwitchDescriptor DescrS9;
bool SaveToDo = false;

/*
 * Fonction de lecture des valeurs dans la memoire flash
 * entrée: structure des parametres
 * sortie: -
*/
void MENU_ReadFlash(S_ParamGen *pParam)
{
    I2C_ReadSEEPROM(pParam, 0, 16);
}
/*
 * Fonction de suvegarde des valeurs dans la memoire flash
 * entrée: structure des parametres
 * sortie: -
*/
void MENU_WriteFlash(S_ParamGen *pParam)
{
    I2C_WriteSEEPROM(pParam, 0, 16);
}
/*
 * Fonction d'initialisation des valeurs
 * entrée: structure des parametres
 * sortie: -
*/
void MENU_Initialize(S_ParamGen *pParam)
{
    //lecture dans la flash
    MENU_ReadFlash(pParam);
    //init système antirebond du switch 9
    DebounceInit(&DescrS9);
}
/*
 * Fonction d'affichage des valeurs et parametres
 * entrée: structure des parametres, element séléctionner, type d'edition
 * sortie: -
*/
void MENU_Display(S_ParamGen *pParam, uint8_t elementSelect, uint8_t menuModeSystem, bool local)
{
    uint8_t charStatue;
    //type de selection
    if(local == false)
        charStatue = '#';
    else if(menuModeSystem == MENU_SELECT)
        //selectionner
        charStatue = '*';
    else if(menuModeSystem ==MENU_EDIT)
        //édition
        charStatue = '?';
    else
        charStatue = ' ';
    //ligne 1
    lcd_gotoxy(1,1);
    if((elementSelect == FORME) || (local == false))
        //affiche avec la selection
        printf_lcd("%cForme =%s ",charStatue,MenuFormes[pParam->Forme]);
    else
        //affichage sans la selection
        printf_lcd(" Forme =%s ",MenuFormes[pParam->Forme]);
    //ligne 2
    lcd_gotoxy(1,2);
    if((elementSelect == FREQUENCE) || (local == false))
        //affiche avec la selection
        printf_lcd("%cFreq [Hz] =   %4d", charStatue, pParam->Frequence);
    else
        //affiche sans la selection
        printf_lcd(" Freq [Hz] =   %4d", pParam->Frequence);
    //ligne 3
    lcd_gotoxy(1,3);
    if((elementSelect == AMPLITUDE) || (local == false))
        //affiche avec la selection
        printf_lcd("%cAmpl [mV] =  %5d", charStatue, pParam->Amplitude);
    else
        //affiche sans la selection
        printf_lcd(" Ampl [mV] =  %5d", pParam->Amplitude);
    //ligne 4
    lcd_gotoxy(1,4);
    if((elementSelect == OFFSET) || (local == false))
        //affiche avec la selection
        printf_lcd("%cOffset [mV] = %4d", charStatue, pParam->Offset);
    else
        //affiche sans la selection
        printf_lcd(" Offset [mV] = %4d", pParam->Offset);
}

/*
 * Fonction qui permet de selectionner quel type d'element séléctionner ou de faire une copie de la valeur avant de faire l'édition de la valeur
 * entrée: structure des parametres, element séléctionner, type d'edition, valeur sauvegarder
 * sortie: 0 rester dans le mode de selection, 1 passage en mode édition de la valeur
*/
uint8_t MENU_Select(S_ParamGen *pParam, uint8_t *elementSelect, int16_t *elementMemory)
{
    //si augementationd de la valeur
    if(Pec12IsPlus())
    {
        //si valeur inferieur a la limite
        if(*elementSelect < OFFSET)
            (*elementSelect)++;
        else
            (*elementSelect) = FORME;
        //reset du flag du PEC12
        Pec12ClearPlus();
    }
    //si diminution de la valeur
    if(Pec12IsMinus())
    {
        //si valeur supérieur a la limite
        if(*elementSelect > FORME)
            (*elementSelect)--;
        else
            (*elementSelect) = OFFSET;
        //reset du flag du PEC12
        Pec12ClearMinus();
    }
    //si passage en mode édition
    if(Pec12IsOK() == 1)
    {
        //sauvegarde de la valeur selectionner
        switch(*elementSelect)
        {
            case FORME:
                *elementMemory =pParam->Forme;
                break;
            case FREQUENCE:
                *elementMemory =pParam->Frequence;
                break;
            case AMPLITUDE:
                *elementMemory =pParam->Amplitude;
                break;
            case OFFSET:
                *elementMemory =pParam->Offset;
                break;
            default:
                break;
        }
        //reset flag du PEC12
        Pec12ClearOK();
        Pec12ClearESC();
        //passage en mode édition
        return 1;
    }
    //reste dans le mode de selection
    return 0;
}

/*
 * Fonction d'édition de la valeur en fonction de l'élément selectionner
 * entrée: structure des parametres, element séléctionner
 * sortie: -
*/
void MENU_Value(S_ParamGen *pParam, uint8_t elementSelect)
{
    switch(elementSelect)
    {
        case FORME:
            //si augementation de la valeur et limite pas attend
            if((Pec12IsPlus()) && (pParam->Forme < SignalCarre))
            {
                pParam->Forme++;
                Pec12ClearPlus();
            }
            //si diminution de la valeur et limite pas attend
            if((Pec12IsMinus()) && (pParam->Forme > SignalSinus))
            {
                pParam->Forme--;
                Pec12ClearMinus();
            }
            break;
        case FREQUENCE:
            //si augementation de la valeur
            if(Pec12IsPlus())
            {
                //si limite pas attend
                if(pParam->Frequence < MAX_FREQUENCE)
                {
                    pParam->Frequence+=PAS_FREQUENCE;
                }
                //limite attend
                else
                {
                    lcd_ClearLine(2);
                    pParam->Frequence = MIN_FREQUENCE;
                }
                Pec12ClearPlus();
            }
            //si diminution de la valeur
            if(Pec12IsMinus())
            {
                //si limite pas attend
                if(pParam->Frequence > MIN_FREQUENCE)
                {
                    pParam->Frequence-=PAS_FREQUENCE;
                }
                //limite attend
                else
                {
                    pParam->Frequence = MAX_FREQUENCE;
                }
                Pec12ClearMinus();
            }
            break;
        case AMPLITUDE:
            //si augementation de la valeur
            if(Pec12IsPlus())
            {
                //si limite pas attend
                if(pParam->Amplitude < MAX_AMPLITUDE)
                {
                    pParam->Amplitude+=PAS_AMPLITUDE;
                }
                //limite attend
                else
                {
                    lcd_ClearLine(3);
                    pParam->Amplitude = MIN_AMPLITUDE;
                }
                Pec12ClearPlus();
            }
            //si diminution de la valeur
            if(Pec12IsMinus())
            {
                //si limite pas attend
                if(pParam->Amplitude > MIN_AMPLITUDE)
                {
                    pParam->Amplitude-=PAS_AMPLITUDE;
                }
                //limite attend
                else
                {
                    pParam->Amplitude = MAX_AMPLITUDE;
                }
                Pec12ClearMinus();
            }
            break;
        case OFFSET:
            //si augementation de la valeur et limite pas attend
            if((Pec12IsPlus()) && (pParam->Offset < MAX_OFFSET))
            {
                pParam->Offset+=PAS_OFFSET;
                Pec12ClearPlus();
            }
            //si diminution de la valeur et limite pas attend
            if((Pec12IsMinus()) && (pParam->Offset > MIN_OFFSET))
            {
                pParam->Offset-=PAS_OFFSET;
                Pec12ClearMinus();
            }
            break;  
        default:
            break;
    }
}

/*
 * Fonction de controle pour passer en mode sauvegarde dans la mémoire flash
 * entrée: -
 * sortie: 0 pas d'appuit, 1 sauvegarder dans la flash, 2 annulation de la sauvegarde 
*/
uint8_t MENU_Save(S_ParamGen *pParam)
{
    static uint16_t counter = 0;
    //si bouton S9 appuier
    if(DebounceIsPressed(&DescrS9) == 1)
    {
        //si l'attente est attend
        if(counter > PUSH_WAIT_SAVE)
        {
            //sauvegarde dans la FLASH
            MENU_WriteFlash(pParam);
            return 1;
        }
        //si bouton relacher
        else if (DebounceIsReleased(&DescrS9) != 0)
            //quitter le mode sans sauvegarde
            return 2;
        //augementation de l'attente
        counter++;
    }
    else
    {
        //reset du conteur
        counter = 0;
        //reset du flag du switch9
        DebounceClearReleased(&DescrS9);
    }
    return 0;
}

/*
 * Fonction de sortie du mode d'édition et de controler si besoin de réstauration de la valeur
 * entrée: structure des parametres, element séléctionner, type d'edition
 * sortie: 1 quitter le mode édition, 0 pas d'interation
*/
uint8_t MENU_Exit(S_ParamGen *pParam, uint8_t elementSelect,  int16_t elementMemory)
{
    //si l'appuie est ok
    if(Pec12IsOK() == 1)
    {
        //quitter le mode édition
        return 1;
    }
    //si l'appuie est ESC
    if(Pec12IsESC() == 1)
    {
        //controle de l'element selectionner
        switch(elementSelect)
        {
            case FORME:
                //reset la ligne
                lcd_ClearLine(1);
                //backup de la valeur
                pParam->Forme = elementMemory;
                break;
            case FREQUENCE:
                //reset la ligne
                lcd_ClearLine(2);
                //backup de la valeur
                pParam->Frequence = elementMemory;
                break;
            case AMPLITUDE:
                //reset la ligne
                lcd_ClearLine(3);
                //backup de la valeur
                pParam->Amplitude = elementMemory;
                break;
            case OFFSET:
                //reset la ligne
                lcd_ClearLine(4);
                //backup de la valeur
                pParam->Offset = elementMemory;
                break;  
        }
        //quitter le mode édition
        return 1;  
    }
    //rester dans le mode édition
    return 0;
}

/*
 * Fonction principale pour la gestion du menu et des sauvegarde
 * entrée: structure des parametres
 * sortie: -
*/
void MENU_Execute(S_ParamGen *pParam,bool local)
{
    static uint16_t timerSave = 0;
    static int16_t elementMemory = 0;
    static uint8_t elementSelect = 0;
    static uint8_t menuModeSystem = MENU_SELECT;
//    static uint8_t triggerUSB = 0;

    DoDebounce (&DescrS9, PORTGbits.RG12);
//    if(triggerUSB != local)
//    {
//        Pec12ClearInactivity();
//        triggerUSB = local;
//    }
        
    switch(menuModeSystem)
    {
        //selection dans le menu
        case MENU_SELECT:
            //controle de la selection de l'éléments et si besoin de modifier la valeur
            if((MENU_Select(pParam, &elementSelect, &elementMemory))&& (local == true))
                menuModeSystem = MENU_EDIT;
            if((local == false) && (SaveToDo))
            {
                MENU_WriteFlash(pParam);
                SaveToDo = false;
            }
                
            //appuie de switch 9
            if((DebounceIsPressed(&DescrS9) == 1) && (local == true))
            {
                //reset l'affichage
                lcd_ClearLine(1);
                lcd_ClearLine(2);
                lcd_ClearLine(3);
                lcd_ClearLine(4);
                //affiche si envie de sauvegarder
                lcd_gotoxy(5,2);
                printf_lcd("Sauvegarde ?");
                lcd_gotoxy(5,3);
                printf_lcd("(appui long)");
                //reset des appuies du switch 9
                DebounceClearPressed(&DescrS9);
                DebounceClearReleased(&DescrS9);
                //passage en mode sauvegarde
                menuModeSystem = MENU_SAVE;
            }
            if(local == false)
            {
                DebounceClearPressed(&DescrS9);
                DebounceClearReleased(&DescrS9);
            }
            break;
            
        //édition de la valeur
        case MENU_EDIT:
            //édition de la valeur
            MENU_Value(pParam, elementSelect);
            //si envie de quitter le mode d'édition
            if(MENU_Exit(pParam, elementSelect, elementMemory))
            {
                //mise à jour du générateur
                GENSIG_UpdatePeriode(pParam);
                GENSIG_UpdateSignal(pParam);
                //reset des switch
                DebounceClearPressed(&DescrS9);
                DebounceClearReleased(&DescrS9);
                Pec12ClearMinus();
                Pec12ClearPlus();
                Pec12ClearOK();
                Pec12ClearESC();
                //passage en mode de selection
                menuModeSystem = MENU_SELECT; 
            }
            break;
            
        //sauvegarde dans la flash
        case MENU_SAVE:
            //si une interaction s'est produit
            switch(MENU_Save(pParam))
            {
                //valeurs sauvegardées
                case 1:
                    //reset de LCD
                    lcd_ClearLine(2);
                    lcd_ClearLine(3);
                    //affichage que la sauvegarde s'est produit
                    lcd_gotoxy(3,2);
                    printf_lcd("Sauvegarde OK !");
                    //passage en mode attente
                    menuModeSystem = MENU_WAIT_SAVE;
                    break;
                //valeurs pas sauvegardées
                case 2:
                    //reset de LCD
                    lcd_ClearLine(2);
                    lcd_ClearLine(3);
                    //affichage que la sauvegarde ne s'est pas produite
                    lcd_gotoxy(1,2);
                    printf_lcd("Sauvegarde ANNULEE !");
                    //passage en mode attente
                    menuModeSystem = MENU_WAIT_SAVE;
                    break;
                default:
                    break;
            }
            //reset de l'attente
            if(menuModeSystem == MENU_WAIT_SAVE)
                timerSave = 0;
            break;
        case MENU_WAIT_SAVE:
            //attend avant de changer de mode
            if(timerSave == TIME_WAIT_SAVE_MS)
            {
                //reset des switchs
                DebounceClearPressed(&DescrS9);
                DebounceClearReleased(&DescrS9);
                Pec12ClearInactivity();
                Pec12ClearMinus();
                Pec12ClearPlus();
                //passage en mode de selection
                menuModeSystem = MENU_SELECT; 
            }
            else
            {
                //augementer le temps de sauvegarde
                timerSave++;
            }
            break;
        default:
            //si un problème est détécté, passage en mode de selection
            menuModeSystem = MENU_SELECT;
            break;
    }
    //si une inactivité est détéctée
    if(Pec12NoActivity() == 0) 
    {
        //si pas en mode de sauvgarde
        if(menuModeSystem < MENU_SAVE)
            //mise à jour de LCD avec les valeurs de parametre
            MENU_Display(pParam, elementSelect, menuModeSystem, local);
        //allumer LCD
        lcd_bl_on();
    }
    //si pas en mode attente de sauvegarde
    else if(menuModeSystem != MENU_WAIT_SAVE)
    {
        //éteindre LCD
        lcd_bl_off();
    }
}

void MENU_DemandeSave(void)
{
    SaveToDo = true;
}