// Canevas manipulation GenSig avec menu
// C. HUBER  09/02/2015
// Fichier Generateur.C
// Gestion  du g�n�rateur

// Pr�vu pour signal de 40 echantillons

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

// Initialisation du  g�n�rateur
void  GENSIG_Initialize(S_ParamGen *pParam)
{
    //Param�tres initiaux
    if(pParam->Magic != 0)
    {
        pParam->Forme = SignalTriangle;
        pParam->Frequence = 1000;     //Hz
        pParam->Amplitude = 5000;   //mV
        pParam->Offset = 0;      //mV   
        pParam->Magic = MAGIC;
    }
    
    //Mise � jour des param�tres initiaux
    GENSIG_UpdatePeriode(pParam);
    GENSIG_UpdateSignal(pParam);  
}
  
// Mise � jour de la periode d'�chantillonage
void  GENSIG_UpdatePeriode(S_ParamGen *pParam)
{
    uint16_t Periode;
    
    //Calcul de la p�riode    
    Periode = (F_SYS/ (pParam->Frequence * MAX_ECH)) - 1 ;
    
    //Mettre la p�riode � travers le driver
    DRV_TMR1_PeriodValueSet(Periode); 
    //PLIB_TMR_Period16BitSet(TMR_ID_3, Periode);
}

// Mise � jour du signal (forme, amplitude, offset)
void  GENSIG_UpdateSignal(S_ParamGen *pParam)
{
    int16_t echNb, offset_mV;
    uint16_t static step;
    
    //Amplitude en mV
    amplitude.mV = ((pParam->Amplitude * Value_UMAX) / Value_MAX_mV);
    
    //Offset en mV
    offset_mV = -(pParam->Offset * (Value_UMAX/ Value_MAX_mV));
 
    //It�ration de 0 � 99
    for (echNb = 0; echNb < MAX_ECH; echNb ++)
    {    
        //Choix du param�tre du signal
        switch (pParam->Forme)
        {
            //Equation pour obtenir un sinus avec l'amplitude et l'offset
            case SignalSinus:
            {                
                //Calcul d'une valeur du signal
                Signal[echNb] = Value_UMOY + amplitude.mV * sin(2* M_PI * echNb / MAX_ECH )  + offset_mV;
              break;
            }
            
            //Calcul pour g�n�rer un triangle
            case SignalTriangle:
            {   
                //Determination de la valeur d'un pas pour un triangle
                step = amplitude.mV * 4 / MAX_ECH;
                
                //La premi�re partie du signal
                if (echNb < 50 )
                {
                    Signal[echNb] = Value_UMOY + amplitude.mV - ( step * echNb) + offset_mV ;
                }
                
                //La deuxi�me partie du signal
                else
                {
                    Signal[echNb] = Value_UMOY - amplitude.mV + ( step * (echNb-50)) + offset_mV;
                }                   
               break;   
            }

            //Calcul pour g�n�rer une dent de Scie en ajoutant l'amplitude et l'offset
            case SignalDentDeScie:
            {           
                //Determination de la valeur d'un pas pour une dent de scie
                step = (amplitude.mV * 2) / MAX_ECH;

                //Calcul d'une valeur du signal
                Signal[echNb] = Value_UMOY + amplitude.mV - ( step * echNb) + offset_mV;
               break;    
            }
            
            //Calcul pour g�n�rer un carr� avec l'amplitude et l'offset
            case SignalCarre:
            {
                //La premi�re partie du signal
                if (echNb < 50 )
                {
                    //Etat haut
                    Signal[echNb] = Value_UMOY - amplitude.mV  + offset_mV ;
                }
                //La deuxi�me partie du signal
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
// Execution du g�n�rateur
// Fonction appel�e dans Int timer3 (cycle variable variable)

// Version provisoire pour test du DAC � modifier
void  GENSIG_Execute(void)
{
   static uint16_t EchNb = 0;
//   const uint16_t Step = 65535 / MAX_ECH;
         
   SPI_WriteToDac(0, Signal[EchNb] );      // sur canal 0
   EchNb++;
   EchNb = EchNb % MAX_ECH;
}