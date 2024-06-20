/*******************************************************************************
  MPLAB Harmony Application Source File
  
  Company:
    Microchip Technology Inc.
  
  File Name:
    appgen.c

  Summary:
    This file contains the source code for the MPLAB Harmony application.

  Description:
    This file contains the source code for the MPLAB Harmony application.  It 
    implements the logic of the application's state machine and it may call 
    API routines of other MPLAB Harmony modules in the system, such as drivers,
    system services, and middleware.  However, it does not call any of the
    system interfaces (such as the "Initialize" and "Tasks" functions) of any of
    the modules in the system or make any assumptions about when those functions
    are called.  That is the responsibility of the configuration-specific system
    files.
 *******************************************************************************/

// DOM-IGNORE-BEGIN
/*******************************************************************************
Copyright (c) 2013-2014 released Microchip Technology Inc.  All rights reserved.

Microchip licenses to you the right to use, modify, copy and distribute
Software only when embedded on a Microchip microcontroller or digital signal
controller that is integrated into your product or third party product
(pursuant to the sublicense terms in the accompanying license agreement).

You should refer to the license agreement accompanying this Software for
additional information regarding your rights and obligations.

SOFTWARE AND DOCUMENTATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY KIND,
EITHER EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION, ANY WARRANTY OF
MERCHANTABILITY, TITLE, NON-INFRINGEMENT AND FITNESS FOR A PARTICULAR PURPOSE.
IN NO EVENT SHALL MICROCHIP OR ITS LICENSORS BE LIABLE OR OBLIGATED UNDER
CONTRACT, NEGLIGENCE, STRICT LIABILITY, CONTRIBUTION, BREACH OF WARRANTY, OR
OTHER LEGAL EQUITABLE THEORY ANY DIRECT OR INDIRECT DAMAGES OR EXPENSES
INCLUDING BUT NOT LIMITED TO ANY INCIDENTAL, SPECIAL, INDIRECT, PUNITIVE OR
CONSEQUENTIAL DAMAGES, LOST PROFITS OR LOST DATA, COST OF PROCUREMENT OF
SUBSTITUTE GOODS, TECHNOLOGY, SERVICES, OR ANY CLAIMS BY THIRD PARTIES
(INCLUDING BUT NOT LIMITED TO ANY DEFENSE THEREOF), OR OTHER SIMILAR COSTS.
 *******************************************************************************/
// DOM-IGNORE-END


// *****************************************************************************
// *****************************************************************************
// Section: Included Files 
// *****************************************************************************
// *****************************************************************************

#include "appgen.h"
#include "Mc32DriverLcd.h"
#include "GesPec12.h"
#include "Generateur.h"
#include "MenuGen.h"
#include "DefMenuGen.h"
#include "Mc32gest_SerComm.h"
#include "Mc32gestI2cSeeprom.h"
#include "Mc32gestSpiDac.h"
#include "Mc32Delays.h"
#include "system_config/pic32mx_eth_sk2/framework/driver/tmr/drv_tmr_static.h"
#include "C:\microchip\harmony\v2_06\bsp\pic32mx_skes\bsp.h"


// *****************************************************************************
// *****************************************************************************
// Section: Global Data Definitions
// *****************************************************************************
// *****************************************************************************

// *****************************************************************************
/* Application Data

  Summary:
    Holds application data

  Description:
    This structure holds the application's data.

  Remarks:
    This structure should be initialized by the APP_Initialize function.
    
    Application strings and buffers are be defined outside this structure.
*/

#define CYCLEAPPDATA    9
#define WAITINIT        3000
#define WAITIP          500

APPGEN_DATA appgenData;

S_ParamGen LocalParamGen;
S_ParamGen RemoteParamGen;

static bool tcpStat = false;
//bool flagIp = false;
static uint16_t waitIP = 0;
bool getTCPMessage = false;

uint8_t readBuffer[APP_READ_BUFFER_SIZE] = "\n";
uint8_t sendBuffer[APP_READ_BUFFER_SIZE] = "\n";

IPV4_ADDR  ipAddr;

// *****************************************************************************
// *****************************************************************************
// Section: Application Callback Functions
// *****************************************************************************
// *****************************************************************************

/* TODO:  Add any necessary callback functions.
*/

void CallbackTimer1()
{
    static uint16_t waitInit = 0;
    static uint8_t cycle = 0;
    
    //mise à jour du PEC12
    //ScanPec12(PORTEbits.RE8,PORTEbits.RE9 ,PORTDbits.RD7);
    ScanPec12(PEC12_A,PEC12_B, PEC12_PB);
    
    //attend le temps de l'init
    if(waitInit == WAITINIT)
    {
        //oscille la LED 1
        BSP_LEDToggle(BSP_LED_1);

        //attend le cycle pour le program
        if(cycle < CYCLEAPPDATA)
            cycle++;
        else
        { 
            //Toggle de la LED 2
            BSP_LEDToggle(BSP_LED_2);
            //change de monde
            appgenData.state = APPGEN_STATE_SERVICE_TASKS;
            cycle = 0;
        }  
    }
    else
        waitInit++;
}

void CallbackTimer3(void)
{
    BSP_LEDOn(BSP_LED_0);
    //mise à jour du générateur
    GENSIG_Execute();
    BSP_LEDOff(BSP_LED_0);
}

// *****************************************************************************
// *****************************************************************************
// Section: Application Local Functions
// *****************************************************************************
// *****************************************************************************


/* TODO:  Add any necessary local functions.
*/


// *****************************************************************************
// *****************************************************************************
// Section: Application Initialization and State Machine Functions
// *****************************************************************************
// *****************************************************************************

/*******************************************************************************
  Function:
    void APPGEN_Initialize ( void )

  Remarks:
    See prototype in appgen.h.
 */

void APPGEN_Initialize ( void )
{
    /* Place the App state machine in its initial state. */
    appgenData.state = APPGEN_STATE_INIT;
    
    DRV_TMR0_Initialize();
    DRV_TMR1_Initialize();
    
    /* TODO: Initialize your application's state machine and other
     * parameters.
     */
}

/******************************************************************************
  Function:
    void APPGEN_Tasks ( void )

  Remarks:
    See prototype in appgen.h.
 */

void APPGEN_Tasks ( void )
{
    /* Check the application's current state. */
    switch ( appgenData.state )
    {
       static bool SaveToDo = false;
       static bool triggerTCP = false;
        
        /* Application's initial state. */
        case APPGEN_STATE_INIT:
        {       
            //Init de LCD
            lcd_init();
            lcd_bl_on();

            // Init SPI DAC
            SPI_InitLTC2604();
            I2C_InitMCP79411();

            // Initialisation PEC12
            Pec12Init();

            // Initialisation du menu
            MENU_Initialize(&LocalParamGen);

            // Initialisation du generateur
            GENSIG_Initialize(&LocalParamGen);
            
            printf_lcd("Tp5 IpGen 2024");
            
            // Affichage
            lcd_gotoxy(1,2);
            printf_lcd("Subramaniyam");
            
            // Active les timers 
            DRV_TMR0_Start();
            DRV_TMR1_Start();
            RemoteParamGen.Frequence = 20;
           
            appgenData.state = APPGEN_STATE_WAIT;

            break;
        }
        
        case APPGEN_STATE_WAIT:
        {
            //Ne rien faire
            break;
        }

        case APPGEN_STATE_SERVICE_TASKS:
        {
            //Execution du menu
            if(tcpStat)
            {
                //si une connection s'est établie
                if(triggerTCP == false)
                {
                    //met a jour l'écran
                    GENSIG_UpdateSignal(&RemoteParamGen);
                    triggerTCP = true;
                }          
                if(getTCPMessage == true)
                {
                    //récupère les information et les copies dans les parametres
                    //et si une demande de sauvegarde est demandé
                    if(GetMessage(readBuffer, &RemoteParamGen, &SaveToDo))
                    {
                        //si demande de sauvegarde est demander
                        if(SaveToDo)
                        {
                            //envoie une demande de sauvegarder dans la mémoire EEPROM
                            MENU_DemandeSave();
                            //ajoute le code magic dans les parametres remote
                            RemoteParamGen.Magic = MAGIC;
                        }
                        //génère le message d'envoie avec les parametres reçu
                        SendMessage(sendBuffer, &RemoteParamGen,SaveToDo);
                        //envoie le message dans ma mémoire d'envoie du TCP
                        SendTCPMessage(sendBuffer);
                        //met a jour les parametres du générateur
                        GENSIG_UpdateSignal(&RemoteParamGen);
                    }
                    //sort du mode inactif
                    Pec12ClearInactivity();
                    //fin de la réception du message
                    getTCPMessage = false;
                }
                //met à jour le système en mode remote
                MENU_Execute(&RemoteParamGen, false);
            }
            //s'il n'est pas connecté
            else
            {     
                if(appgenData.newIp == true)
                {
                    if(waitIP == WAITIP)
                    {
                        appgenData.newIp = false;
                        waitIP = 0;
//                        tcpStat = true;
                    }
                    else
                    {
                        waitIP++;
                    }
                }
                if(triggerTCP == true)
                {
                    //met a jour le générateur
                    GENSIG_UpdateSignal(&LocalParamGen);
                    //fin de la détéction TCP
                    triggerTCP = false;
                }
                if(waitIP == 0)
                //met a jour le system en mode local
                MENU_Execute(&LocalParamGen, true);
   
            } 
            appgenData.state = APPGEN_STATE_WAIT;
            
            break;
        }

        /* The default state should never be executed. */
        default:
        {
            /* TODO: Handle error in application's state machine. */
            break;
        }
    }
}

void APP_UpdateStateTCP ( bool connectionState )
{
    tcpStat = connectionState;
}

void APPGEN_DispNewAddress(IPV4_ADDR ipAddr)
{
    //appgenData.newIp = true;
    lcd_ClearLine(1);
    lcd_ClearLine(2);
    lcd_ClearLine(3);
    lcd_ClearLine(4);
    
    //affichage adr. IP   
    lcd_gotoxy(8,2);
    printf_lcd("Adr. IP");
    lcd_gotoxy(2,3);
    printf_lcd("IP:%03d.%03d.%03d.%03d ", ipAddr.v[0], ipAddr.v[1], ipAddr.v[2], ipAddr.v[3]); 
}
/*******************************************************************************
 End of File
 */
