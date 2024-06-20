// Mc32Gest_SerComm.C
// fonction d'�mission et de r�ception des message
// transmis en USB CDC
// Canevas TP4 SLO2 2015-2015


#include "app.h"
#include "Mc32gest_SerComm.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>



// Fonction de reception  d'un  message
// Met � jour les param�tres du generateur a partir du message recu
// Format du message
//  !S=TF=2000A=10000O=+5000W=0#
//  !S=PF=2000A=10000O=-5000D=100W=1#

//USBReadBuffer : buffer de donn�e
//pParam : les paramettres du g�n�rateur
//commande de sauvegarde dans la flash

bool GetMessage(uint8_t *USBReadBuffer, S_ParamGen *pParam, bool *SaveTodo)
{
    uint8_t Indexparametre = 0;
    uint8_t IndexArrayChar = 6;
    char valueparameter[6];
    //r�cup�ration de la forme du signal
    switch(USBReadBuffer[3])
    {
        case 'S':
            pParam->Forme = SignalSinus;
            break;
        case 'T':
            pParam->Forme = SignalTriangle;
            break;
        case 'D':
            pParam->Forme = SignalDentDeScie;
            break;
        case 'C':
            pParam->Forme = SignalCarre;
            break;
        default:
            return false;
            break;
    }
    //copie la valeur de la fr�quence
    while((USBReadBuffer[Indexparametre+6] <= 0x39) && (Indexparametre < 6))
    {
        valueparameter[Indexparametre] =USBReadBuffer[Indexparametre+6];
        Indexparametre++;
    }
    //si pas d'erreur d�t�cter
    if(Indexparametre>0)
    {
        //copie la valeur dans les parametres
        pParam->Frequence = atoi(valueparameter);
        if(pParam->Frequence > MaxFrequence)
            pParam->Frequence = MaxFrequence;
        else if(pParam->Frequence < MinFrequence)
            pParam->Frequence = MinFrequence;
    }
        
    else
        return false;
    //met � jour l'index g�n�ral
    IndexArrayChar += Indexparametre+2;
    Indexparametre = 0;
    //copie la valeur de l'amplitude dans une chaine de charact�re
    while((USBReadBuffer[Indexparametre+IndexArrayChar] <= 0x39) && (Indexparametre < 6))
    {
        valueparameter[Indexparametre] =USBReadBuffer[Indexparametre+IndexArrayChar];
        Indexparametre++;
    }
    //met uen fin de chaine de caract�re
    valueparameter[Indexparametre+1] =0x00;
    //si pas d'erreur d�t�ct�
    if(Indexparametre>0)
    {
        //copie la valeur dans les parametres et controle la limite
        pParam->Amplitude = atoi(valueparameter);
        if(pParam->Amplitude > MaxAmplitude)
            pParam->Amplitude = MaxAmplitude;
        else if(pParam->Amplitude < MinAmplitude)
            pParam->Amplitude = MinAmplitude;
    }
    else
        return false;        
    //met � jour l'index g�n�ral
    IndexArrayChar += Indexparametre+2;
    Indexparametre = 1;
    valueparameter[0] =USBReadBuffer[IndexArrayChar];
    //copie la valeur de l'offset dans une chaine de charact�re
    while((USBReadBuffer[Indexparametre+IndexArrayChar] <= 0x39) && (Indexparametre < 6))
    {
        valueparameter[Indexparametre] =USBReadBuffer[Indexparametre+IndexArrayChar];
        Indexparametre++;
    }
    //met uen fin de chaine de caract�re
    valueparameter[Indexparametre] =0x00;
    //si pas d'erreur d�t�cter
    if(Indexparametre>0)
    {
        pParam->Offset = atoi(valueparameter);
        if(pParam->Offset > MaxOffset)
            pParam->Offset = MaxOffset;
        else if(pParam->Offset < MinOffset)
            pParam->Offset = MinOffset;
    }
    else
        return false;
    //met � jour l'index g�n�ral
    IndexArrayChar += Indexparametre+2;
    //check s'il y a une demande de sauvegarde
    if(USBReadBuffer[IndexArrayChar] == '1')
        *SaveTodo = true;
    else
        *SaveTodo = false;
    //retourne qu'un message est bien re�u correctement
    return true;
} // GetMessage


// Fonction d'envoi d'un  message
// Rempli le tampon d'�mission pour USB en fonction des param�tres du g�n�rateur
// Format du message
// !S=TF=2000A=10000O=+5000D=25WP=0#
// !S=TF=2000A=10000O=+5000D=25WP=1#    // ack sauvegarde



void SendMessage(uint8_t *USBSendBuffer, S_ParamGen *pParam, bool Saved )
{
    char FormeSignal = ' ';
    char SaveInFlash = ' ';
    char SignOffset = ' ';
    int OffsetValue = 0;
    //converti la valeur du type de signal en char
    switch(pParam->Forme)
    {
        case SignalSinus:
            FormeSignal = 'S';
            break;
        case SignalTriangle:
            FormeSignal = 'T';
            break;
        case SignalDentDeScie:
            FormeSignal = 'D';
            break;
        case SignalCarre:
            FormeSignal = 'C';
            break;
        default:
            break;
    }
    //sauvegarde le signe
    if(pParam->Offset < 0)
    {
        SignOffset = '-';
        OffsetValue = (int)(pParam->Offset * -1);
    }
    else
    {
        SignOffset = '+';
        OffsetValue = (int)pParam->Offset;
    }
    //s'il y a eu une sauvegarde
    if(Saved)
        SaveInFlash = '1';
    else
        SaveInFlash = '0';
    //copie les valeurs des parametres dans une chaine de charact�re
    sprintf( (char*)USBSendBuffer, "!S=%cF=%dA=%dO=%c%dWP=%c#", FormeSignal, (int)pParam->Frequence, (int)pParam->Amplitude,SignOffset, OffsetValue, SaveInFlash );
} // SendMessage
