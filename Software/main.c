/*
 * BME 464 - EMG Decoder
 *
 * Decodes signals from two EMG leads and sends result to an external interface
 * 
 */

#include "Lcd.h"
#include <delays.h> //delay header
#include <p18f46k22.h> //chip header
#include <stdio.h> //enable 

#pragma config FOSC = INTIO67   // Internal OSC block, Port Function on RA6/7
// #pragma config WDTEN = SWON     // Watch Dog Timer controlled by SWDTEN bit
#pragma config WDTEN = OFF      // Watch Dog Timer disabled. SWDTEN no effect
// #pragma config WDTPS = 512      // WDT Postscale of 1:512, equivalent to 2 secs
#pragma config XINST = OFF      // Instruction set Extension and indexed Addressing mode disabled

//Define statements
#define Running 0
#define Sleeping 1 

//Variable definitions
unsigned int V1; // signal from lead 1
unsigned int V2; // signal from lead 2
unsigned int DV1; // debounced signal from lead 1
unsigned int DV2; // debounced signal from lead 2
unsigned int oldDV1; // old debounced signal from lead 1
unsigned int oldDV2; // old debounced signal from lead 2
int Thresh; // threshold for voltage classification
int Hyst; // hysteresis range
int EMG; // decoded result
int State;
int max;
int min;
int recComm; // received signal via communication

//Function definitions
void SysInit(void);
void GetData(void);
unsigned int DebounceChan(unsigned int value, unsigned int old);
int Decode(unsigned int voltage1, unsigned int voltage2);
void Transmit(int info);
void SleepMode(void);
//Test functions
void Lights(void);

void main(void)
{
     // Local variables
    char str[4];
    //Initialize
    SysInit();
    LCDClear();

    // EMG Decoder Loop
    while(1) {
        
        // For EMG decoding
		// if (State == Running) { 
            GetData(); // Acquire voltages
            EMG=Decode(DV1,DV2); // Decode
            Lights();
            //Transmit(EMG); // Print value to screen or communication
            
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
        LCDGoto(8,1);
        LCDPutByte(EMG);
            
            Delay10KTCYx(10);  // Delay 1/10 second
		// }
		// if (State == Sleeping){
           // SleepMode();      
		// }
         
		/*
        // For Communication Testing
		TXREG1=60; // Set info to be transmitted
		Delay10KTCYx(50); // Delay 2 seconds        
		recComm=RCREG1; // Read info received       
		LCDGoto(0,0);
		LCDPutByte(recComm);
		*/
	};
}

//Initialize necessary systems
void SysInit(void)
{
    OSCCON=0b01010110; //4 MHz internal oscillator

    //Set up LEDs
    ANSELB=0b00000000; //Digital IO
    LATB=0b00000000; //LEDs off
    TRISB=0b00000000; //LEDs are outputs
    
    //Set up ADC channel on RA1
    ANSELAbits.ANSA1 = 1;
    TRISAbits.RA1 = 1; //Analog in

    //Set up ADC channel on RA0 
    ANSELAbits.ANSA0 = 1;
    TRISAbits.RA0 = 1; //Analog in

    //Set up ADC parameters
    ADCON2bits.ACQT=001; //2 TAD
    ADCON2bits.ADCS=010; //FOSC/32
    ADCON2bits.ADFM=1; //Right justified***
    ADCON0bits.ADON=1; //Turn on A/D

	//Set up sleep mode
	OSCCONbits.IDLEN = 0;

	//Set up Watchdog Timer
	// WDTCONbits.SWDTEN = 1; // Turn on WDT

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
	V1=0;
	V2=0;
	EMG=0;
	State=0;
	min = 0;
	max = 0;
	recComm = 0;
	//Set threshold & hysteresis variable
	DV1=0;
    DV2=0;
    oldDV1=0;
    oldDV2=0;
    Thresh=300; //Initial threshold
    Hyst=100; //Hysteresis width
}

// ADC sampling of EMG leads
void GetData(void) {
	
	//Channel1
	ADCON0bits.CHS=0001; //Select RA1
	ADCON0bits.GO=1; //Start conversion
    while(ADCON0bits.GO==1){}; //Wait for finish
    V1=ADRESH;
    V1=(V1<<8) | ADRESL; //Make 10-bit
    //Process voltage from channel 1
    DV1=DebounceChan(V1,oldDV1);
    oldDV1=DV1;

    //Channel2
    ADCON0bits.CHS=0000; //Select RA2
	ADCON0bits.GO=1; //Start conversion
    while(ADCON0bits.GO==1){}; //Wait for finish
    V2=ADRESH;
    V2=(V2<<8) | ADRESL; //Make 10-bit
    //Process voltage from channel 2
    DV2=DebounceChan(V2,oldDV2);
    oldDV2=DV2;

}

//Hysteresis for each channel
unsigned int DebounceChan(unsigned int value, unsigned int old){
// If the level was low before, raise the threshold
if (old==0){
    //process if voltage is lower or higher than threshold
    if (value>=Thresh){
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
}



// Decode the two digital signals
int Decode(unsigned int voltage1, unsigned int voltage2){
	//Decode based on binary inputs
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
    // For now, display EMG
    LCDClear();
    LCDGoto(0,0);
    LCDPutByte(EMG);
    
    State++;
}

void SleepMode(void){
    LCDClear();
    LCDWriteStr("SLEEP MODE");
    Sleep();
    State = 0;  
}

//Test functions
void Lights(void){
	//Turn all LEDs off
	LATB=0b00000000; //LEDs off
    switch(EMG){
		//Turn on based on EMG
		case 3:
		LATBbits.LATB3=1;
		break;
		case 2:
		LATBbits.LATB2=1;
		break;
		case 1:
		LATBbits.LATB1=1;
		break;
		case 0:
		LATBbits.LATB0=1;
		break;
	}
}
