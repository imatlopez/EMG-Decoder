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
int thres1; // magnitude threshold
int thres2; // magnitude threshold
int rslope1; // rising slope threshold
int dslope1; // descending slope threshold
int rslope2; // rising slope threshold
int dslope2; // descending slope threshold
int hyst; // hysteresis range
int state;
int t; // sampling period (x10^-2 seconds)

//Function definitions
void SysInit(void);
void Calibrate(void);
int GetData(int channel);
int Debounce(int raw, int oldraw, int olddeb, int channel);
int Decode(int channel1, int channel2);
void Transmit(int info);

void main(void)
{
     // Local variables
	int rawV1; // raw signal from lead 1
	int rawV2; // raw signal from lead 2
	int V1; // processed signal from lead 1
	int V2; // processed signal from lead 2
	int EMG; // decoded result

    //Initialize
    SysInit();
    LCDClear();
    Calibrate();

    // EMG Decoder Loop
    while(1) {
        // oscillate
		rawV1 = GetData(1); // Acquire voltage from channel 1
		rawV2 = GetData(2); // Acquire voltage from channel 2
		V1=Debounce(rawV1, oldRawV1,oldV1,1); //Process voltage from channel 1
		oldRawV1=rawV1;
		oldV1=V1;
		V2=Debounce(rawV2,oldRawV2,oldV2,2); //Process voltage from channel 2
		oldRawV2=rawV2;
		oldV2=V2;
        /*
		if( ((oldV1==0 && V1==1) || (oldV2==0 && V2==1)) && !(V1==1 && V2==1)){ // need to debounce simultaneous contractions
			Delay10KTCYx(t);  // Delay a period
			continue;
		}
         */
		EMG=Decode(V1,V2); // Decode
        Transmit(EMG); // Print value to screen or communication
		Delay10KTCYx(t);  // Delay t/100 seconds
	};
}

//Initialize necessary systems
void SysInit(void)
{
    OSCCON=0b01010110; //4 MHz internal oscillator

    //Set up ADC channel on RA2
    TRISA=0x04;
    ANSELAbits.ANSA2 = 1;
    TRISAbits.RA2 = 1; //Analog in

    //Set up ADC channel on RA3 
    ANSELAbits.ANSA3 = 1;
    TRISAbits.RA3 = 1; //Analog in
    
    //Set up ADC channel on RA1 
    ANSELAbits.ANSA1 = 0;
    TRISAbits.RA1 = 0; //Digital out

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

	//Configure variables (adjustable)
    //thres=300; //Initial threshold
    //hyst=100; //Hysteresis width
	t=20; //200 ms (1 corresponds to 10 ms)
}

//Calibration period to adjust for user/muscle activity
void Calibrate(void){
	int i;
	int old1;
	int old2;
	int raw1;
	int raw2;
	int max1;
	int min1;
	int max2;
	int min2;
	int slope;
	int rising1;
	int descend1;
	int rising2;
	int descend2;
    char str[4];
    // Initialize calibration
	LCDGoto(5,0);
	LCDPutByte(0);
    Delay10KTCYx(1000); //Wait 10 seconds for system to settle
	old1=0;
	old2=0;
    max1=0;
    min1=1023;
    max2=0;
    min2=1023;
	rising1=0;
	descend1=0;
	rising2=0;
	descend2=0;
    // Calibrate mag and slope thresholds
	LCDGoto(5,0);
	LCDPutByte(1);
    for(i=0;i<50;i++){ //10 seconds of calibration data
    	//Channel 1 magnitude calibration
    	raw1=GetData(1); // Acquire voltage from channel 1
    	if(raw1>max1){
    		max1=raw1;
    	}
    	if(raw1<min1){
    		min1=raw1;
    	}
		// slope calibration
		slope = raw1-old1;
		if(slope>rising1)
			rising1 = slope;
		if(slope<descend1)
			descend1 = slope;
		old1 = raw1;
    	//Channel 2 magnitude calibration
		raw2=GetData(2); // Acquire voltage from channel 2
    	if(raw2>max2){
    		max2=raw2;
    	}
    	if(raw2<min2){
    		min2=raw2;
		}
		// slope calibration
		slope = raw2-old2;
		if(slope>rising2)
			rising2 = slope;
		if(slope<descend2)
			descend2 = slope;
		old2 = raw2;
        /*
        //Display raw value for Ch1
        LCDGoto(8,0);
		sprintf(str,"%04u",raw1);
        LCDPutChar(str[0]);
        LCDPutChar(str[1]);
        LCDPutChar(str[2]);
        LCDPutChar(str[3]);
        //Display raw value for Ch2
        LCDGoto(8,1);
        sprintf(str,"%04u",raw2); 
        LCDPutChar(str[0]);
        LCDPutChar(str[1]);
        LCDPutChar(str[2]);
        LCDPutChar(str[3]);
        */
		Delay10KTCYx(t);
    }
    LCDGoto(5,0);
	LCDPutByte(2);
    //Need to figure this out
    thres1=min1+(max1-min1)*3/10;
    thres2=min2+(max2-min2)*3/10;
	rslope1 = rising1*0.9;
	dslope1 = descend1*0.9;
	rslope2 = rising2*0.9;
	dslope2 = descend2*0.9;
    descend1 = -1*descend1;
    //Ch1 mag thres
    LCDGoto(0,0);
	sprintf(str,"%04u", min1);
    LCDPutChar(str[0]);
    LCDPutChar(str[1]);
    LCDPutChar(str[2]);
    LCDPutChar(str[3]);
    //Ch2 mag thres
	LCDGoto(0,1);
    sprintf(str,"%04u",max1); 
    LCDPutChar(str[0]);
    LCDPutChar(str[1]);
    LCDPutChar(str[2]);
    LCDPutChar(str[3]);
    LCDGoto(8,0);
    sprintf(str,"%04u",rising1); 
    LCDPutChar(str[0]);
    LCDPutChar(str[1]);
    LCDPutChar(str[2]);
    LCDPutChar(str[3]);
    	LCDGoto(8,1);
    sprintf(str,"%04u",descend1); 
    LCDPutChar(str[0]);
    LCDPutChar(str[1]);
    LCDPutChar(str[2]);
    LCDPutChar(str[3]);
}

// ADC sampling of EMG leads
int GetData(int channel) {
	int volt;
    //char str[4];
	if(channel==1){
		//Channel 1
		ADCON0bits.CHS=2; //Select RA0
		ADCON0bits.GO=1; //Start conversion
		while(ADCON0bits.GO==1){}; //Wait for finish
		volt=ADRESH;
		volt=(volt<<8) | ADRESL; //Make 10-bit
	}
	if(channel==2){
		//Channel2
		ADCON0bits.CHS=3; //Select RA1
		ADCON0bits.GO=1; //Start conversion
		while(ADCON0bits.GO==1){}; //Wait for finish
		volt=ADRESH;
		volt=(volt<<8) | ADRESL; //Make 10-bit
	}
    /*
	LCDGoto(8,channel-1);
	sprintf(str,"%04u",volt); 
	LCDPutChar(str[0]);
	LCDPutChar(str[1]);
    LCDPutChar(str[2]);
    LCDPutChar(str[3]);
     */
	return volt;
}

// Process signal from each channel into either high or low
int Debounce(int raw, int oldraw, int olddeb, int channel){
    int thres;
	int slope;
	int risThres;
	int desThres;
	//Set threshold
	if(channel==1){
		thres=thres1;
		risThres=rslope1;
		desThres=dslope1;
	}
	if(channel==2){
		thres=thres2;
	    risThres=rslope2;
		desThres=dslope2;
	}
	// Find instantaneous slope
	slope = raw-oldraw; // unit: voltage units
	// Threshold is 15 V/s * 1023 units/5V = 3069 units/sec = 3.069 units/ms
	// slopeThres = 3.069;
	// If the signal was low before:
	if (olddeb==0){
		if ((slope>risThres) || (raw>thres)){
			return 1;
		}
		else{
			return 0;
		}
	}
	// If the level was high before:
    if (olddeb==1){
		if((slope<0 && raw<thres)){ //(slope<desThres) || (slope<0 && raw<Thresh)
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
	//LCDGoto(13,1);
	//LCDPutByte(info);
	// Transmit EMG
	TXREG1 = info; // Set info to be transmitted
}