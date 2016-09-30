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
#pragma config WDTEN = OFF      // Watch Dog Timer disabled. SWDTEN no effect
#pragma config XINST = OFF      // Instruction set Extension and indexed Addressing mode disabled

//Define statements
 #define Running 0

//Variable definitions
unsigned int V1; // signal from lead 1
unsigned int V2; // signal from lead 2
int EMG; // decoded result

//Function definitions
void SysInit(void);
void GetData(void);
int Decode(unsigned int voltage1, unsigned int voltage2);
void Transmit(int info);

void main(void)
{
    // Local variables
    char str[4];
	
    //Initialize
    SysInit();
    LCDClear();

    // EMG Decoder Loop
    while(1) {

		GetData(); // Acquire voltages
		EMG=Decode(V1,V2); // Decode
		Transmit(EMG); // Print value to screen or communication

		// For now, display the two voltage results
		LCDGoto(0,0);
		sprintf(str,"%04u",V1);
        LCDPutChar(str[0]);
        LCDPutChar(str[1]);
        LCDPutChar(str[2]);
        LCDPutChar(str[3]);
        LCDGoto(0,1);
        sprintf(str,"%04u",V2); 
        LCDPutChar(str[0]);
        LCDPutChar(str[1]);
        LCDPutChar(str[2]);
        LCDPutChar(str[3]);
        Delay10KTCYx(200);           // Delay 2 seconds
		
	};
}

//Initialize necessary systems
void SysInit(void)
{
    OSCCON=0b01010110; //4 MHz internal oscillator

    //Set up buttons
    ANSELBbits.ANSB0=0; //Digital
    TRISAbits.RA4=1; //Input
    TRISBbits.RB0=1; //Input

    //Set up ADC channel on AN0
    ANSELAbits.ANSA0 = 1;
    TRISAbits.RA0 = 1; //Analog in

    //Set up ADC channel on AN1 
    ANSELAbits.ANSA1 = 1;
    TRISAbits.RA1 = 1; //Analog in

    //Set up ADC parameters
    ADCON2bits.ACQT=001; //2 TAD
    ADCON2bits.ADCS=010; //FOSC/32
    ADCON2bits.ADFM=1; //Right justified***
    ADCON0bits.ADON=1; //Turn on A/D

    //Set up LCD
    ANSELD = 0x00;
    TRISD = 0x00; //Digital out
    LCDInit(); //Start LCD
    LCDWriteStr("Starting device...");

	//Reset variables
	V1=0;
	V2=0;
	EMG=0;
}


// ADC sampling of EMG leads
void GetData(void)
{
	//Channel1
	ADCON0bits.CHS=0000; //Select RA0
	ADCON0bits.GO=1; //Start conversion
    while(ADCON0bits.GO==1){}; //Wait for finish
    V1=ADRESH;
    V1=(V1<<8) | ADRESL; //Math needs to be done in the int variable
    if(V1==1023) //Fix roundoff error
            V1=1022;
    //Channel2
    ADCON0bits.CHS=0001; //Select RA1
	ADCON0bits.GO=1; //Start conversion
    while(ADCON0bits.GO==1){}; //Wait for finish
    V2=ADRESH;
    V2=(V2<<8) | ADRESL; //Math needs to be done in the int variable
    if(V2==1023) //Fix roundoff error
            V2=1022;
}

// Decode the two digital signals
int Decode(unsigned int voltage1, unsigned int voltage2){
	return 0;
}

// Transmit the result to external interface
void Transmit(int info){

}

