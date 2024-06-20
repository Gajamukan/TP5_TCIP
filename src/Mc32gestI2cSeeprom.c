//--------------------------------------------------------
// Mc32gestI2cEEprom.C
//--------------------------------------------------------
// Gestion I2C de la SEEPROM du MCP79411 (Solution exercice)
//	Description :	Fonctions pour EEPROM MCP79411
//
//	Auteur 		: 	C. HUBER
//      Date            :       26.05.2014
//	Version		:	V1.0
//	Compilateur	:	XC32 V1.31
// Modifications :
//
/*--------------------------------------------------------*/

#include "Mc32gestI2cSeeprom.h"
#include "Mc32_I2cUtilCCS.h"


// Définition pour MCP79411
#define MCP79411_EEPROM_R    0xAF         // MCP79411 address for read
#define MCP79411_EEPROM_W    0xAE         // MCP79411 address for write
// La EEPROM du 79411 est de 1 Kbits donc 128 octets
#define MCP79411_EEPROM_BEG   0x00         // addr. début EEPROM
#define MCP79411_EEPROM_END   0x7F         // addr. fin EEPROM

// Definitions du bus (pour mesures)
// #define I2C-SCK  SCL2/RA2      PORTAbits.RA2   pin 58
// #define I2C-SDA  SDa2/RA3      PORTAbits.RA3   pin 59




// Initialisation de la communication I2C et du MCP79411
// ------------------------------------------------

void I2C_InitMCP79411(void)
{
   bool Fast = true;
   i2c_init( Fast );
   
} //end I2C_InitMCP79411

// Ecriture d'un bloc dans l'EEPROM du MCP79411 
void I2C_WriteSEEPROM(void *SrcData, uint32_t EEpromAddr, uint16_t NbBytes)
{
    uint8_t cycle = 0;
    uint8_t pageCycle = 0;
    uint8_t *i2cData = SrcData;
    i2c_start();
    //send page
    while((NbBytes-cycle) > NBR_PAGE_I2C)
    {
        //start le message
        while(!i2c_write(MCP79411_EEPROM_W))
            i2c_start();
        //envoie l'adresse de ou on veut écrire
        i2c_write((uint8_t)EEpromAddr);
        pageCycle= 0;
        //envoie 8 byte sur I2C
        while(pageCycle<NBR_PAGE_I2C)
        {
            //copie la case du tableau et envoie dans l'I2C
            i2c_write(i2cData[cycle]);
            //met a jours les index
            cycle++;
            pageCycle++;
        }
        //passe au page suivant
        EEpromAddr+=NBR_PAGE_I2C;
        //fin du message
        i2c_stop();
    }
    do
    {
        //début du message
        i2c_start();
    }while(!i2c_write(MCP79411_EEPROM_W));
    //envoit la dernière adresse
    i2c_write((uint8_t)EEpromAddr);
    while(cycle < NbBytes)
    {
        //copie la case du tableau et envoie dans I2C
        i2c_write(i2cData[cycle]);
        //met a jour l'index
        cycle++;
    }
    //fin de la communication I2C
    i2c_stop();
} // end I2C_WriteSEEPROM

// Lecture d'un bloc dans l'EEPROM du MCP79411
void I2C_ReadSEEPROM(void *DstData, uint32_t EEpromAddr, uint16_t NbBytes)
{
    uint8_t cycle = 0;
    uint8_t *i2cData = DstData;
    //début du message
    i2c_start();
    //commande écriture
    while(!i2c_write(MCP79411_EEPROM_W))
        i2c_start();
    //envoie l'adresse de ou on veut lire
    i2c_write((uint8_t)EEpromAddr);
    //début de la réception
    i2c_reStart();
    //envoie la commande de lecture
    i2c_write(MCP79411_EEPROM_R);
    //envoie tout les byte a mémoriser
    for(cycle = 0; cycle < NbBytes-1; cycle++)
    {
        //copie la valeur dans le tableau avec un ack
        i2cData[cycle]=i2c_read(1);
    }
    //copie la dernière valeur sans le ack
    i2cData[NbBytes-1]=i2c_read(0);
    //fin de réception I2C
    i2c_stop();
} // end I2C_ReadSEEPROM