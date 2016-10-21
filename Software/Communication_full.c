/* * BME 464 - EMG Decoder * * Decodes signals from two EMG leads and sends result to an external interface *  */#include "Lcd.h"#include <delays.h> //delay header#include <p18f46k22.h> //chip header#include <stdio.h>#pragma config FOSC = INTIO67   // Internal OSC block, Port Function on RA6/7#pragma config WDTEN = OFF      // Watch Dog Timer disabled. SWDTEN no effect#pragma config XINST = OFF      // Instruction set Extension and indexed Addressing mode disabled//Variable definitionschar a;//Function definitionsvoid SysInit(void);void Transmit(int info);void main(void){    // Local variables    char str[4];     //Initialize    SysInit();    LCDClear();    // EMG Decoder Loop    while(1) {        TXREG1=98;        Delay10KTCYx(50);           // Delay 2 seconds        a=RCREG1;        LCDClear();        LCDGoto(0,0);  LCDPutByte(a);        Delay10KTCYx(50);          LCDClear();        LCDGoto(0,0);  LCDPutByte(1); };}//Initialize necessary systemsvoid SysInit(void){    OSCCON=0b01010110; //4 MHz internal oscillator    //Set up LCD    ANSELD = 0x00;    TRISD = 0x00; //Digital out    LCDInit(); //Start LCD    LCDWriteStr("Starting device...");    //Set up serial    TRISCbits.RC6 = 1; //TX pin set as output    TRISCbits.RC7 = 1; //RX pin set as input (may not need)    //Set baudrate to 10417    SPBRG1=5; //Set to 5    TXSTA1bits.BRGH=0; //Use low     BAUDCON1bits.BRG16=0; //Use 8-bit transmission    TXSTA1bits.SYNC=0; //Use asynch mode    RCSTA1bits.SPEN=1; //Enable USART    TXSTA1bits.TXEN=1; //Enable transmission        //RX    ANSELC=0x00;    RCSTA1bits.CREN=1; //Enable receiver        a=0;}
