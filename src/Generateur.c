// Canevas manipulation GenSig avec menu
// C. HUBER  09/02/2015
// Fichier Generateur.C
// Gestion  du générateur

// Prévu pour signal de 40 echantillons

// Migration sur PIC32 30.04.2014 C. Huber


#include "Generateur.h"
#include "DefMenuGen.h"
#include "Mc32gestSpiDac.h"
#include "driver/tmr/drv_tmr_static.h"

// T.P. 2016 100 echantillons
#define MAX_ECH 99
#define F_SYS   80000000
#define Value_UMAX 65535
#define Value_UMOY 32768
#define Value_MAX_mV 20000

S_Amplitude amplitude;
int32_t Signal[MAX_ECH];

// Initialisation du  générateur
void  GENSIG_Initialize(S_ParamGen *pParam)
{
    //Paramètres initiaux
    if(pParam->Magic != 0)
    {
        pParam->Forme = SignalTriangle;
        pParam->Frequence = 1000;     //Hz
        pParam->Amplitude = 5000;   //mV
        pParam->Offset = 0;      //mV   
        pParam->Magic = MAGIC;
    }
    
    //Mise à jour des paramètres initiaux
    GENSIG_UpdatePeriode(pParam);
    GENSIG_UpdateSignal(pParam);  
}
  
// Mise à jour de la periode d'échantillonage
void  GENSIG_UpdatePeriode(S_ParamGen *pParam)
{
    uint16_t Periode;
    
    //Calcul de la période    
    Periode = (F_SYS/ (pParam->Frequence * MAX_ECH)) - 1 ;
    
    //Mettre la période à travers le driver
    DRV_TMR1_PeriodValueSet(Periode); 
    //PLIB_TMR_Period16BitSet(TMR_ID_3, Periode);
}

// Mise à jour du signal (forme, amplitude, offset)
void  GENSIG_UpdateSignal(S_ParamGen *pParam)
{
    int16_t echNb, offset_mV;
    uint16_t static step;
    
    //Amplitude en mV
    amplitude.mV = ((pParam->Amplitude * Value_UMAX) / Value_MAX_mV);
    
    //Offset en mV
    offset_mV = -(pParam->Offset * (Value_UMAX/ Value_MAX_mV));
 
    //Itération de 0 à 99
    for (echNb = 0; echNb < MAX_ECH; echNb ++)
    {    
        //Choix du paramètre du signal
        switch (pParam->Forme)
        {
            //Equation pour obtenir un sinus avec l'amplitude et l'offset
            case SignalSinus:
            {                
                //Calcul d'une valeur du signal
                Signal[echNb] = Value_UMOY + amplitude.mV * sin(2* M_PI * echNb / MAX_ECH )  + offset_mV;
              break;
            }
            
            //Calcul pour générer un triangle
            case SignalTriangle:
            {   
                //Determination de la valeur d'un pas pour un triangle
                step = amplitude.mV * 4 / MAX_ECH;
                
                //La première partie du signal
                if (echNb < 50 )
                {
                    Signal[echNb] = Value_UMOY + amplitude.mV - ( step * echNb) + offset_mV ;
                }
                
                //La deuxième partie du signal
                else
                {
                    Signal[echNb] = Value_UMOY - amplitude.mV + ( step * (echNb-50)) + offset_mV;
                }                   
               break;   
            }

            //Calcul pour générer une dent de Scie en ajoutant l'amplitude et l'offset
            case SignalDentDeScie:
            {           
                //Determination de la valeur d'un pas pour une dent de scie
                step = (amplitude.mV * 2) / MAX_ECH;

                //Calcul d'une valeur du signal
                Signal[echNb] = Value_UMOY + amplitude.mV - ( step * echNb) + offset_mV;
               break;    
            }
            
            //Calcul pour générer un carré avec l'amplitude et l'offset
            case SignalCarre:
            {
                //La première partie du signal
                if (echNb < 50 )
                {
                    //Etat haut
                    Signal[echNb] = Value_UMOY - amplitude.mV  + offset_mV ;
                }
                //La deuxième partie du signal
                else
                {
                    //Etat bas
                    Signal[echNb] = Value_UMOY + amplitude.mV + offset_mV;
                }                 
              break; 
            }

            default : 
            {
              break;    
            }       
        }
        //Limitation de l'amplitude minimum et maximum
        if (Signal[echNb] < 0 )
        {
            Signal[echNb] = 0;
        }
        else if (Signal[echNb] > Value_UMAX )
        {
            Signal[echNb] = Value_UMAX;
        }        
    }
}
// Execution du générateur
// Fonction appelée dans Int timer3 (cycle variable variable)

// Version provisoire pour test du DAC à modifier
void  GENSIG_Execute(void)
{
   static uint16_t EchNb = 0;
//   const uint16_t Step = 65535 / MAX_ECH;
         
   SPI_WriteToDac(0, Signal[EchNb] );      // sur canal 0
   EchNb++;
   EchNb = EchNb % MAX_ECH;
}