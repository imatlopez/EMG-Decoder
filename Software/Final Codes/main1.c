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
// #define Running 0
// #define Sleeping 1 

//Variable definitions
int oldRawV1; // old raw signal from lead 1
int oldRawV2; // old raw signal from lead 2
int oldV1; // old processed signal from lead 1
int oldV2; // old processed signal from lead 2
int thres1; // threshold for voltage classification
int thres2; // threshold for voltage classification
int hyst; // hysteresis range
int state;
int max1;
int min1;
int max2;
int min2;
int recComm; // received signal via communication
int t; // sampling period (x10^-2 seconds)

//Function definitions
void SysInit(void);
int GetData(int channel);
int Debounce(int raw, int oldraw, int olddeb, int channel);
int Decode(int channel1, int channel2);
void Transmit(int info);
void Calibrate(void);

void main(void)
{
     // Local variables
	int rawV1; // raw signal from lead 1
	int rawV2; // raw signal from lead 2
	int V1; // processed signal from lead 1
	int V2; // processed signal from lead 2
	int EMG; // decoded result
	int count;

    //Initialize
    SysInit();
    LCDClear();
    Calibrate();

    // EMG Decoder Loop
    while(1) {
		rawV1 = GetData(1); // Acquire voltage from channel 1
		rawV2 = GetData(2); // Acquire voltage from channel 2
		V1=Debounce(rawV1, oldRawV1,oldV1,1); //Process voltage from channel 1
		oldRawV1 = rawV1;
		oldV1=V1;
		V2=Debounce(rawV2,oldRawV2,oldV2,2); //Process voltage from channel 2
		oldRawV2 = rawV2;
		oldV2=V2;
		EMG=Decode(V1,V2); // Decode
        Transmit(EMG); // Print value to screen or communication
		Delay10KTCYx(t);  // Delay t/100 seconds
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

    //Set up ADC channel on RA0
    ANSELAbits.ANSA0 = 1;
    TRISAbits.RA1 = 1; //Analog in

    //Set up ADC channel on RA1 
    ANSELAbits.ANSA1 = 1;
    TRISAbits.RA0 = 1; //Analog in

    //Set up ADC parameters
    ADCON2bits.ACQT=001; //2 TAD
    ADCON2bits.ADCS=010; //FOSC/32
    ADCON2bits.ADFM=1; //Right justified***
    ADCON0bits.ADON=1; //Turn on A/D

	//Set up sleep mode
	// OSCCONbits.IDLEN = 0;

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
	oldRawV1=0;
	oldRawV2=0;
	oldV1=0;
	oldV2=0;
	state=0;
	thres1=0;
	thres2=0;

	//Configure variables (adjustable)
    //thres=300; //Initial threshold
    // hyst=100; //Hysteresis width
	t=20; //200 ms (1 corresponds to 10 ms)
}

void Calibrate(void){
	int count;
	int ca1;
	int ca2;
    char str[4];
    //Calibration period
    // Display EMG
	LCDGoto(5,0);
	LCDPutByte(0);
    Delay10KTCYx(1000); //Wait 10 seconds for system to settle
    max1=0;
    min1=1023;
    max2=0;
    min2=1023;
    // Display EMG
	LCDGoto(5,0);
	LCDPutByte(8);
    for(count=0;count<200;count++){ //Calibrate for 10 seconds
    	//Ch1
    	ca1=GetData(1); // Acquire voltage from channel 1
    	if(ca1>max1){
    		max1=ca1;
    	}
    	if(ca1<min1){
    		min1=ca1;
    	}
    	//Ch2
		ca2=GetData(2); // Acquire voltage from channel 2
    	if(ca2>max2){
    		max2=ca2;
    	}
    	if(ca2<min2){
    		min2=ca2;
    	}
        LCDGoto(8,0);
		sprintf(str,"%04u",ca1);
        LCDPutChar(str[0]);
        LCDPutChar(str[1]);
        LCDPutChar(str[2]);
        LCDPutChar(str[3]);
        //Raw value for Ch1
        LCDGoto(8,1);
        sprintf(str,"%04u",ca2); 
        LCDPutChar(str[0]);
        LCDPutChar(str[1]);
        LCDPutChar(str[2]);
        LCDPutChar(str[3]);
    Delay10KTCYx(5);
    }
    LCDGoto(5,0);
	LCDPutByte(1);
    //need to figure this out
    thres1=min1+(max1-min1)*3/10;
    thres2=min2+(max2-min2)*3/10;
    //Raw value for Ch1
    	LCDGoto(0,0);
		sprintf(str,"%04u",thres1);
        LCDPutChar(str[0]);
        LCDPutChar(str[1]);
        LCDPutChar(str[2]);
        LCDPutChar(str[3]);
        //Raw value for Ch1
        LCDGoto(0,1);
        sprintf(str,"%04u",thres2); 
        LCDPutChar(str[0]);
        LCDPutChar(str[1]);
        LCDPutChar(str[2]);
        LCDPutChar(str[3]);
}

// ADC sampling of EMG leads
int GetData(int channel) {
	int volt;
    char str[4];
	if(channel==1){
		//Channel 1
		ADCON0bits.CHS=0000; //Select RA0
		ADCON0bits.GO=1; //Start conversion
		while(ADCON0bits.GO==1){}; //Wait for finish
		volt=ADRESH;
		volt=(volt<<8) | ADRESL; //Make 10-bit
	}
	if(channel==2){
		//Channel2
		ADCON0bits.CHS=0001; //Select RA1
		ADCON0bits.GO=1; //Start conversion
		while(ADCON0bits.GO==1){}; //Wait for finish
		volt=ADRESH;
		volt=(volt<<8) | ADRESL; //Make 10-bit
	}
        LCDGoto(8,channel-1);
        sprintf(str,"%04u",volt); 
        LCDPutChar(str[0]);
        LCDPutChar(str[1]);
        LCDPutChar(str[2]);
        LCDPutChar(str[3]);
	return volt;
}

// Process signal from each channel into either high or low
int Debounce(int raw, int oldraw, int olddeb, int channel){
    int thres;
	float slope;
	float slopeThres;
	//Set threshold
	if(channel=1){
		thres=thres1;
	}
	if(channel=2){
		thres=thres2;
	}

	// Find instantaneous slope
	slope = (raw-oldraw)/(t*10.0); // unit: voltage units/ms
	// Threshold is 15 V/s * 1023 units/5V = 3069 units/sec = 3.069 units/ms
	slopeThres = 3.069;
	// If the signal was low before:
	if (olddeb==0){
		if ((slope>slopeThres) || (raw>thres)){
			return 1;
		}
		else{
			return 0;
		}
	}
	// If the level was high before:
    if (olddeb==1){
		if((slope<0 && raw<thres)){ //(slope<-1*slopeThresh) || (slope<0 && raw<Thresh)
            return 0;
        }
        else{
            return 1;
        }
    }
}

// Decode the two digital signals
int Decode(int channel1, int channel2){
	if (channel1==1){
        if(channel2==1){
            return 3; //11
        }
        else {
            return 2; //10
        }
    }
    else {
        if(channel2==1){
            return 1; //01
        }
        else {
            return 0; //00
        }
    }
}

// Transmit the result to external interface
void Transmit(int info){
    // Display EMG
	LCDGoto(13,1);
	LCDPutByte(info);
	// Transmit EMG
	TXREG1 = info;
}

