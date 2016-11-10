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
#pragma config WDTEN = OFF      // Watch Dog Timer disabled. SWDTEN no effect
// #pragma config WDTPS = 512      // WDT Postscale of 1:512, equivalent to 2 secs
#pragma config XINST = OFF      // Instruction set Extension and indexed Addressing mode disabled

//Define statements
#define Running 0
#define Sleeping 1 

//Variable definitions
int V1; // signal from lead 1
int V2; // signal from lead 2
int oldV1; // old signal from lead 1
int oldV2; // old signal from lead 2
int active; //Is there activity?
int Thresh; // threshold for voltage classification
int Hyst; // hysteresis range
int EMG; // decoded result
int max;
int min;
int recComm; // received signal via communication
int t;
//Function definitions
void SysInit(void);
void GetData(void);
int Detect();
int Decode();
void Transmit(int info);
void SleepMode(void);
//Test functions
void Lights(void);
void Zero(void);

void main(void)
{

    //Initialize
    SysInit();
    LCDClear();

    // EMG Decoder Loop
    while(1) {
        
            active=Detect();// Check if there's any input
			if(active==1){
				Zero();
				EMG=Decode(); // Decode
				Transmit(EMG); // Print value to screen or communication
				Delay10KTCYx(200);  // Delay for period of robot movement (2 seconds)
				Zero(); //Reset variables
				Transmit(EMG);
			}
         
            Delay10KTCYx(t);  // Delay 1/10 second --> this is sampling period
 
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
	V1=0;
	V2=0;
	EMG=0;
	min = 0;
	max = 0;
	recComm = 0;
	//Set threshold & hysteresis variable
    oldV1=0;
    oldV2=0;
    Thresh=250; //Initial threshold
    Hyst=100; //Hysteresis width
	t= 5 ; // 50 ms (1 corresponds to 10 ms)
}

// ADC sampling of EMG leads
void GetData(void) {
    char str[4];
	//Save old values
	oldV1=V1;
	oldV2=V2;

	//Channel1
	ADCON0bits.CHS=0001; //Select RA1
	ADCON0bits.GO=1; //Start conversion
    while(ADCON0bits.GO==1){}; //Wait for finish
    V1=ADRESH;
    V1=(V1<<8) | ADRESL; //Make 10-bit
	
    //Channel2
    ADCON0bits.CHS=0000; //Select RA2
	ADCON0bits.GO=1; //Start conversion
    while(ADCON0bits.GO==1){}; //Wait for finish
    V2=ADRESH;
    V2=(V2<<8) | ADRESL; //Make 10-bit

    //Display raw values

    	//Raw value for Ch1
    	LCDGoto(0,0);
		sprintf(str,"%04u",V1);
        LCDPutChar(str[0]);
        LCDPutChar(str[1]);
        LCDPutChar(str[2]);
        LCDPutChar(str[3]);
        //Raw value for Ch1
        LCDGoto(0,1);
        sprintf(str,"%04u",V2); 
        LCDPutChar(str[0]);
        LCDPutChar(str[1]);
        LCDPutChar(str[2]);
        LCDPutChar(str[3]);
}

//Hysteresis for each channel
int Detect(void){
	float slope1;
	float slope2;
	float slopeThres;
	// Sample data
	GetData();

	//For Ch1
	// Find instantaneous slope
	slope1 = (V1-oldV1)/(t*10); // units: unit/ms
	slope2 = (V2-oldV2)/(t*10); // units: unit/ms
	// Threshold is 15 V/s * 1023 units/5V = 3069 units/sec = 3.069 units/ms
	slopeThres = 3.069;
	if ((slope1>slopeThres) || (V1>Thresh)){
		return 1;
	}
	else{
		if ((slope2>slopeThres) || (V2>Thresh)){
			return 1;
		}
		else{
			return 0;
		}
	}
}


// Check against amplitude threshold
int Check(int raw){
	if(raw>Thresh){
        return 1;
    }
    else{
        return 0;
	}
}


// Decode the two digital signals
int Decode(void){
	int result;
	Delay10KTCYx(30);  // Wait for 500 ms after onset of activity
	GetData(); //Sample first set 
	Delay10KTCYx(70);  // Wait for 500 ms after onset of activity
	GetData(); //Sample second set

	//Determine whether  from oldV1, V1, oldV2, V2
	oldV1=Check(oldV1);
	V1=Check(V1);
	oldV2=Check(oldV2);
	V2=Check(V2);
	result = 8*oldV1+4*V1+2*oldV2+V2;
    return result;
}
 
// Transmit the result to external interface
void Transmit(int info){
    // For now, display EMG
        //Debounced value for Ch1
        LCDGoto(6,0);
        LCDPutByte(oldV1);
        LCDGoto(9,0);
        LCDPutByte(V1);
        //Debounced value for Ch2
        LCDGoto(6,1);
        LCDPutByte(oldV2);
        LCDGoto(9,1);
        LCDPutByte(V2);
        //Display command output
        LCDGoto(12,1);
        LCDPutByte(EMG);
}

void Zero(void){
	oldV1=0;
	oldV2=0;
	V1=0;
	V2=0;
	EMG=0;
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
