/*
 * BME 464 - Robotic arm controller
 *
 * Receives command from EMG decoder and uses result to control robotic arm
 * 
 */

#include "Lcd.h"
#include <delays.h> //delay header
#include <p18f46k22.h> //chip header
#include <stdio.h>

#pragma config FOSC = INTIO67   // Internal OSC block, Port Function on RA6/7
#pragma config WDTEN = OFF      // Watch Dog Timer disabled. SWDTEN no effect
#pragma config XINST = OFF      // Instruction set Extension and indexed Addressing mode disabled

//Variable definitions
int EMG; // decoded result
int O1;
int O2;
int O3;

//Function definitions
void SysInit(void);

void main(void)
{
    //Initialize
    SysInit();
    LCDClear();

    // EMG Decoder Loop
    while(1) {
        // Communication   
		EMG=RCREG1; // Read info received       
		LCDGoto(0,0);
		LCDPutByte(EMG);
        switch(EMG){
		case 0: //resting
			O1=0;
			O2=0;
			O3=0;
            break;
		case 1: //
			O1=1;
			O2=0;
			O3=0;
            break;
		case 2: //
			O1=0;
			O2=1;
			O3=0;
            break;
		case 3: //
			O1=0;
			O2=0;
			O3=1;
            break;
		case 80: //System settling
			LCDGoto(0,0);
			LCDWriteStr("Wait");
			break;
		case 81: //Calibrating
			LCDGoto(0,0);
			LCDWriteStr("Calibrate");
		case 82: //Ready
			LCDGoto(0,0);
			LCDClear();	
		}
        if(O1==0)
			PORTAbits.RA0=0;
		else
			PORTAbits.RA0=1;
		if(O2==0)
			PORTAbits.RA1=0;
		else
			PORTAbits.RA1=1;
		if(O3==0)
			PORTAbits.RA2=0;
		else
			PORTAbits.RA2=1;
		Delay10KTCYx(20);  // Delay 1/5 second
	}
}

//Initialize necessary systems
void SysInit(void)
{
    OSCCON=0b01010110; //4 MHz internal oscillator

    //Set up buttons
    ANSELAbits.ANSA0=0; //Digital
    ANSELAbits.ANSA1=0; //Digital
    ANSELAbits.ANSA2=0; //Digital
    ANSELAbits.ANSA3=0; //Digital
    TRISAbits.RA0=0; //Output
    TRISAbits.RA1=0; //Output
    TRISAbits.RA2=0; //Output
    TRISAbits.RA3=0; //Output
    
    //Set up LCD
    ANSELD = 0x00;
    TRISD = 0x00; //Digital out
    LCDInit(); //Start LCD
    LCDWriteStr("Starting device...");

	//Set up async EUSART    
	TRISCbits.RC6 = 1; //Initialize TX pin (transmission) 
	TRISCbits.RC7 = 1; //Initialize RX pin (reception)
	TXSTA1bits.BRGH=0; //Use low     
	BAUDCON1bits.BRG16=0; //Use 8-bit transmission 
	SPBRG1=5;   //Set baud rate to 10417    
	TXSTA1bits.SYNC=0; //Specify async mode    
	RCSTA1bits.SPEN=1; //Enable EUSART    
	TXSTA1bits.TXEN=1; //Enable transmission
    ANSELC=0x00;           
	RCSTA1bits.CREN=1; //Enable receiver
	
	//Reset variables
	EMG=0;
	O1=0;
	O2=0;
	O3=0;
}
