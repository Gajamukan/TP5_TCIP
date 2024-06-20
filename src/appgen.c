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

APPGEN_DATA appgenData;

S_ParamGen LocalParamGen;
S_ParamGen RemoteParamGen;

bool TcpStat ;

//uint8_t readBuffer[APP_READ_BUFFER_SIZE] = "\n";
//uint8_t sendBuffer[APP_READ_BUFFER_SIZE] = "\n";

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
    ScanPec12(PORTEbits.RE8,PORTEbits.RE9 ,PORTDbits.RD7);
    
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
//       static bool SaveToDo = false;
//       static bool triggerUSB = false;
        
        /* Application's initial state. */
        case APPGEN_STATE_INIT:
        {       
            //Init de LCD
            lcd_init();
            lcd_bl_on();

            //Init SPI DAC
            SPI_InitLTC2604();
            
            //Init I2C
            I2C_InitMCP79411();

            // Initialisation PEC12
            Pec12Init();

            // Initialisation du menu
            MENU_Initialize(&LocalParamGen);

            RemoteParamGen = LocalParamGen;
            
            // Initialisation du generateur
            GENSIG_Initialize(&LocalParamGen);
            
//            //mise à jour du signal
//            GENSIG_UpdateSignal(&LocalParamGen);
//            //mettre à jour la période
//            GENSIG_UpdatePeriode(&LocalParamGen);
            
            // Active les timers 
            DRV_TMR0_Start();
            DRV_TMR1_Start();
            
            printf_lcd("TP5 IpGen 2024");
            lcd_gotoxy(1,2); 
            printf_lcd("Subra");

            //RemoteParamGen.Frequence = 20;
           
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
//            if (TcpStat == false)
//            {
//                //met à jour le générateur
//                GENSIG_UpdateSignal(&LocalParamGen);
                
                // Execution du menu en local
                MENU_Execute(&LocalParamGen,true);
                
//                I2C_WriteSEEPROM(&RemoteParamGen, 0, sizeof(S_ParamGen)); 
//                I2C_ReadSEEPROM(&LocalParamGen, 0 , sizeof(S_ParamGen));
//            }
//            else
//            {
//                //met a jour l'écran
//                GENSIG_UpdateSignal(&RemoteParamGen);
//                
//                //Execution du menu en remote
//                MENU_Execute(&RemoteParamGen,false);
//            }
                
            appgenData.state = APPGEN_STATE_WAIT;
            
            if(appgenData.newIp == true)
            {
//                appgenData.newIp == false;
                lcd_gotoxy(1,1);
                printf_lcd("") ;
                lcd_gotoxy(1,2);
                printf_lcd("") ;
            
            }
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

//void APPGEN_TCP( uint8_t *Buffer )
//{    
//    /* Flag if save needed */
//    bool saveToDo = false;
//    
//    /* Get message from UART */
//    GetMessage(Buffer, &RemoteParamGen, &saveToDo);
//    
//    GENSIG_UpdateSignal(&RemoteParamGen);
//
//    if(saveToDo)
//    {
//        I2C_WriteSEEPROM(&RemoteParamGen, MCP79411_EEPROM_BEG, sizeof(S_ParamGen));
//        
//        lcd_ClearLine(1);
//        lcd_ClearLine(2);
//        lcd_ClearLine(3);
//        lcd_ClearLine(4);
//        
//        lcd_gotoxy(4, 2);
//        printf_lcd("Sauvegarde OK");
//        
//        delay_ms(800);
//                
//        MENU_Initialize(&RemoteParamGen);
//    }
//    
//    SendMessage(Buffer, &RemoteParamGen, saveToDo);
//}

void APPGEN_DispNewAddress(IPV4_ADDR ipAddr)
{
    appgenData.newIp = true;
    
    lcd_gotoxy(1,4);
    printf_lcd("IP:%03d.%03d.%03d.%03d ", ipAddr.v[0], ipAddr.v[1], ipAddr.v[2], ipAddr.v[3]);
}

/*******************************************************************************
 End of File
 */
