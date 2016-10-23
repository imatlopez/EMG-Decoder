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
// #pragma config WDTEN = SWON     // Watch Dog Timer controlled by SWDTEN bit
#pragma config WDTEN = OFF      // Watch Dog Timer disabled. SWDTEN no effect
// #pragma config WDTPS = 512      // WDT Postscale of 1:512, equivalent to 2 secs
#pragma config XINST = OFF      // Instruction set Extension and indexed Addressing mode disabled

//Define statements
#define Running 0
#define Sleeping 1 

//Variable definitions
unsigned int voltVals1[3]; // signal from lead 1
unsigned int voltVals2[3]; // signal from lead 2
int i; // counter 
unsigned int V1; // moving average signal from lead 1
unsigned int V2; // moving average signal from lead 2
int EMG; // decoded result
int State;
int max;
int min;
int recComm; // received signal via communication

//Function definitions
void SysInit(void);
void GetData(void);
int Decode(unsigned int voltage1, unsigned int voltage2);
void Transmit(int info);
void SleepMode(void);

void main(void)
{
    //Initialize
    SysInit();
    LCDClear();

    // EMG Decoder Loop
    while(1) {
		// if (State == Running) { 
            GetData(); // Acquire voltages
            EMG=Decode(V1,V2); // Decode
            Transmit(EMG); // Print value to screen or communication
            Delay10KTCYx(10);  // Delay 1/10 second
		// }
		// if (State == Sleeping){
           // SleepMode();      
		// }

		/* // For Communication Testing
		TXREG1=98;        // Set info to be transmitted
		Delay10KTCYx(50);           // Delay 2 seconds        
		recComm=RCREG1;        // Read info received       
		LCDGoto(0,0);  
		LCDPutByte(recComm);        
		*/
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

    //Set up ADC channel on RA1
    ANSELAbits.ANSA1 = 1;
    TRISAbits.RA1 = 1; //Analog in

    //Set up ADC channel on RA2 
    ANSELAbits.ANSA2 = 1;
    TRISAbits.RA2 = 1; //Analog in

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
	RCSTA1bits.CREN=1; //Enable receiver
	
	//Reset variables
	i=0;
	V1=0;
	V2=0;
	EMG=0;
	State=0;
	min = 0;
	max = 0;
	recComm = 0;
}

// ADC sampling of EMG leads
void GetData(void) {
	unsigned int volt;
	//Channel1
	ADCON0bits.CHS=0001; //Select RA1
	ADCON0bits.GO=1; //Start conversion
    while(ADCON0bits.GO==1){}; //Wait for finish
    volt=ADRESH;
    volt=(volt<<8) | ADRESL; //Math needs to be done in the int variable
    if(volt==1023) //Fix roundoff error
            volt=1022;
	voltVals1[i] = volt;
    //Channel2
    ADCON0bits.CHS=0010; //Select RA2
	ADCON0bits.GO=1; //Start conversion
    while(ADCON0bits.GO==1){}; //Wait for finish
    volt=ADRESH;
    volt=(volt<<8) | ADRESL; //Math needs to be done in the int variable
    if(volt==1023) //Fix roundoff error
            volt=1022;
	voltVals2[i] = volt;
	i++;
	if(i>2)
		i=0;
	//Find moving average
	V1=(voltVals1[0]+voltVals1[1]+voltVals1[2])/3;
	V2 = (voltVals2[0]+voltVals2[1]+voltVals2[2])/3;
}

// Decode the two digital signals
int Decode(unsigned int voltage1, unsigned int voltage2){
	int thres;
	if(voltage1>max)
		max = voltage1;
	if(voltage1<min)
		min = voltage1;
	thres = (min+max)/2;
	if(voltage1>thres){
        PORTBbits.RB0=1;
		return 1;
    }
	else{
        PORTBbits.RB0=0;
		return 0;
    }
}

// Transmit the result to external interface
void Transmit(int info){
    // Local variables
    char str[4];
	
    // For now, display the two voltage results
    LCDClear();
    LCDGoto(0,0);
	sprintf(str,"%04u",V1);
    LCDPutChar(str[0]);
    LCDPutChar(str[1]);
    LCDPutChar(str[2]);
    LCDPutChar(str[3]);
    LCDGoto(0,1);
    LCDPutByte(EMG);
    
    State++;
}

void SleepMode(void){
    LCDClear();
    LCDWriteStr("SLEEP MODE");
    Sleep();
    State = 0;  
}

