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
unsigned int PV1; // processed signal from lead 1
unsigned int PV2; // processed signal from lead 2
unsigned int oldPV1; // processed signal from lead 1
unsigned int oldPV2; // processed signal from lead 2
int Thresh; // threshold for voltage classification
int Hyst; // hysteresis range
int EMG; // decoded result

//Function definitions
void SysInit(void);
void GetData(void);
void Process(void);
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
		EMG=Decode(PV1,PV2); // Decode
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
        //Display command output
        LCDGoto(0,5);
        LCDPutByte(EMG);
        Delay10KTCYx(50);           // Delay .5 seconds
		
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
    PV1=0;
    PV2=0;
    oldPV1=0;
    oldPV2=0;
    //Set threshold & hysteresis
    Thresh=512;
    Hyst=50;
}


// ADC sampling of EMG leads
void GetData(void)
{
	//Channel1
	ADCON0bits.CHS=0000; //Select RA0
	ADCON0bits.GO=1; //Start conversion
    while(ADCON0bits.GO==1){}; //Wait for finish
    V1=ADRESH;
    V1=(V1<<8) | ADRESL; //Covert output to 10-bit
    // add iteration here if want to do averaging
    //Process voltage from channel 1
    VP1=Process(V1,oldPV1);


    //Channel2
    ADCON0bits.CHS=0001; //Select RA1
	ADCON0bits.GO=1; //Start conversion
    while(ADCON0bits.GO==1){}; //Wait for finish
    V2=ADRESH;
    V2=(V2<<8) | ADRESL; ///Covert output to 10-bit
    VP2=Process(V2,oldPV2)

}

unsigned int Process(unsigned int value, unsigned int old);
{
// If the level was low before, raise the threshold
if (old==0){
    //process if voltage is lower or higher than threshold
    if (value>=Thresh+Hyst){
        return 1;
    }
    else{
        return 0;
    }
}
// If the level was high before, lower the threshold
else{
    if (value>=Thresh-Hyst){
        return 1;
    }
    else{
        return 0;
    }
}

// Decode the two digital signals
int Decode(unsigned int voltage1, unsigned int voltage2){
	if (voltage1==1){
        if(voltage2==1){
            return 3; //11
        }
        else {
            return 2; //10
        }
    }
    else {
        if(voltage2==1){
            return 1; //01
        }
        else {
            return 0; //00
        }
    }
}

// Transmit the result to external interface
void Transmit(int info){

}

