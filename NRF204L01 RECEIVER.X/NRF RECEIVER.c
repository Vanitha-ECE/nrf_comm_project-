
// PIC16F877A Configuration Bit Settings

// 'C' source line config statements

// CONFIG
#pragma config FOSC = EXTRC     // Oscillator Selection bits (RC oscillator)
#pragma config WDTE = OFF       // Watchdog Timer Enable bit (WDT disabled)
#pragma config PWRTE = OFF      // Power-up Timer Enable bit (PWRT disabled)
#pragma config BOREN = OFF      // Brown-out Reset Enable bit (BOR disabled)
#pragma config LVP = OFF        // Low-Voltage (Single-Supply) In-Circuit Serial Programming Enable bit (RB3 is digital I/O, HV on MCLR must be used for programming)
#pragma config CPD = OFF        // Data EEPROM Memory Code Protection bit (Data EEPROM code protection off)
#pragma config WRT = OFF        // Flash Program Memory Write Enable bits (Write protection off; all program memory may be written to by EECON control)
#pragma config CP = OFF         // Flash Program Memory Code Protection bit (Code protection off)

// #pragma config statements should precede project file includes.
// Use project enums instead of #define for ON and OFF.

#include <xc.h>
#define _XTAL_FREQ 8000000

#define CE  RC0
#define CSN RC1

void SPI_Init() 
{
    TRISC3 = 0; 
    TRISC4 = 1; 
    TRISC5 = 0; 
    SSPSTAT = 0b01000000;
    SSPCON = 0b00100000;
}

void SPI_Write(unsigned char data) 
{
    SSPBUF = data;
    while(!SSPSTATbits.BF);
}

unsigned char SPI_Read() 
{
    SPI_Write(0xFF);
    return SSPBUF;
}

void NRF_WriteReg(unsigned char reg, unsigned char value) 
{
    CSN = 0;
    SPI_Write(0x20 | reg);
    SPI_Write(value);
    CSN = 1;
}

unsigned char NRF_ReadReg(unsigned char reg) 
{
    unsigned char result;
    CSN = 0;
    SPI_Write(reg & 0x1F);
    result = SPI_Read();
    CSN = 1;
    return result;
}

void NRF_ReadPayload(unsigned char *data, unsigned char len) 
{
    CSN = 0;
    SPI_Write(0x61); 
    for(unsigned char i = 0; i < len; i++) {
        data[i] = SPI_Read();
    }
    CSN = 1;
}

void NRF_InitRX() 
{
    CE = 0;
    CSN = 1;
    __delay_ms(100);

    NRF_WriteReg(0x00, 0x0B); 
    NRF_WriteReg(0x01, 0x00); 
    NRF_WriteReg(0x02, 0x01); 
    NRF_WriteReg(0x03, 0x03); 
    NRF_WriteReg(0x04, 0x00); 
    NRF_WriteReg(0x05, 76);   
    NRF_WriteReg(0x06, 0x06); 
    NRF_WriteReg(0x11, 5);    

    CE = 1; 
}

void main() 
{
    TRISC0 = 0; 
    TRISC1 = 0; 
    TRISB0 = 0; 

    SPI_Init();
    NRF_InitRX();

    unsigned char buffer[5];

    while(1) {
        if (NRF_ReadReg(0x07) & 0x40) 
        {
            NRF_ReadPayload(buffer, 5); 
            PORTB = buffer[0];          
            NRF_WriteReg(0x07, 0x40);   
        }
    }
}



