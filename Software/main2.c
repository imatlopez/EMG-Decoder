/*
 * BME 464 - EMG Decoder
 *
 * Decodes signals from two EMG leads and sends result to an external interface
 * 
 */

#include "Lcd.h"
#include <delays.h> //delay header
#include <p18f46k22.h> //chip header
#include <stdio.h>

#pragma config FOSC = INTIO67   // Internal OSC block, Port Function on RA6/7
#pragma config WDTEN = OFF      // Watch Dog Timer disabled. SWDTEN no effect
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
        // For Communication Testing     
		EMG=RCREG1; // Read info received       
		LCDGoto(0,0);
		LCDPutByte(EMG);
		switch(EMG){
		case '0': //resting
			O1=0;
			O2=0;
			O3=0;
		case '1': //
			O1=1;
			O2=0;
			O3=0;
		case '2':
			O1=0;
			O2=1;
			O3=0;
		case '3':
			O1=0;
			O2=0;
			O3=1;
		}
		PORTBbits.RB0=O1;
		PORTBbits.RB0=O2;
		PORTBbits.RB0=O3;
		Delay10KTCYx(10);  // Delay 1/10 second
	};
}

//Initialize necessary systems
void SysInit(void)
{
    OSCCON=0b01010110; //4 MHz internal oscillator

    //Set up buttons
    ANSELBbits.ANSB0=0; //Digital
    ANSELCbits.ANSC2=0; //Digital
    TRISAbits.RA4=1; //Input
    TRISBbits.RB0=0; //Output
    TRISCbits.RC2=0; //Output

    //Set up LCD
    ANSELD = 0x00;
    TRISD = 0x00; //Digital out
    LCDInit(); //Start LCD
    LCDWriteStr("Starting device...");

	//Set up async EUSART    
	TRISCbits.RC6 = 1; //Initialize TX pin (transmission) 
	TRISCbits.RC7 = 1; //Initialize RX pin (reception)
	TXSTA1bits.BRGH=0; //Use low     
	BAUDCON1bits.BRG16=0; //Use 8-bit transmission 
	SPBRG1=5;   //Set baud rate to 10417    
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

