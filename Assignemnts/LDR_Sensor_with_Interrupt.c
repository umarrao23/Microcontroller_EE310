//------------------------------------------------------------------------------
// Title:LDR-Based Intruder Alert System with Interrupt Wait Mode
//------------------------------------------------------------------------------
// Purpose:   detects ambient light level using an LDR and displays the value
//            in lux on an LCD. If an interrupt is triggered eg a switch),
//            the system enters a "WAIT" state, blinking an LED for 10 seconds.
//
// Special features:
//   - Uses ADC to read voltage from LDR and converts to approximate lux
//   - Displays real-time lux readings on a 16x2 LCD
//   - External interrupt (RB1) triggers a "WAITTTT" mode with blinking LED
//   - LED connected to RB0 when pressed it displays wait for 10 seconds
//   - System resumes normal LDR display after wait
//
// Compiler : MPLAB X IDE v6.2, XC8 Compiler
// MCU      : PIC18F47K42
// Author   : Umar Wahid
// Date     : May 2025
// Inputs   : LDR Sensor on RA0 (AN0), Interrupt Button on RB1
// Outputs  : LCD on RD0-RD7 (EN RC2 and RS RC3)
// Version  : 1.0
//------------------------------------------------------------------------------



#include <xc.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "lcd.h"

#pragma config FEXTOSC = LP     // External Oscillator Selection (LP (crystal oscillator) optimized for 32.768 kHz; PFM set to low power)
#pragma config RSTOSC = EXTOSC  // Reset Oscillator Selection (EXTOSC operating per FEXTOSC bits (device manufacturing default))

// CONFIG1H
#pragma config CLKOUTEN = OFF   // Clock out Enable bit (CLKOUT function is disabled)
#pragma config PR1WAY = ON      // PRLOCKED One-Way Set Enable bit (PRLOCK bit can be cleared and set only once)
#pragma config CSWEN = ON       // Clock Switch Enable bit (Writing to NOSC and NDIV is allowed)
#pragma config FCMEN = ON       // Fail-Safe Clock Monitor Enable bit (Fail-Safe Clock Monitor enabled)

// CONFIG2L
#pragma config MCLRE = EXTMCLR  // MCLR Enable bit (If LVP = 0, MCLR pin is MCLR; If LVP = 1, RE3 pin function is MCLR )
#pragma config PWRTS = PWRT_OFF // Power-up timer selection bits (PWRT is disabled)
#pragma config MVECEN = ON      // Multi-vector enable bit (Multi-vector enabled, Vector table used for interrupts)
#pragma config IVT1WAY = ON     // IVTLOCK bit One-way set enable bit (IVTLOCK bit can be cleared and set only once)
#pragma config LPBOREN = OFF    // Low Power BOR Enable bit (ULPBOR disabled)
#pragma config BOREN = SBORDIS  // Brown-out Reset Enable bits (Brown-out Reset enabled , SBOREN bit is ignored)

// CONFIG2H
#pragma config BORV = VBOR_2P45 // Brown-out Reset Voltage Selection bits (Brown-out Reset Voltage (VBOR) set to 2.45V)
#pragma config ZCD = OFF        // ZCD Disable bit (ZCD disabled. ZCD can be enabled by setting the ZCDSEN bit of ZCDCON)
#pragma config PPS1WAY = ON     // PPSLOCK bit One-Way Set Enable bit (PPSLOCK bit can be cleared and set only once; PPS registers remain locked after one clear/set cycle)
#pragma config STVREN = ON      // Stack Full/Underflow Reset Enable bit (Stack full/underflow will cause Reset)
#pragma config DEBUG = OFF      // Debugger Enable bit (Background debugger disabled)
#pragma config XINST = OFF      // Extended Instruction Set Enable bit (Extended Instruction Set and Indexed Addressing Mode disabled)

// CONFIG3L
#pragma config WDTCPS = WDTCPS_31// WDT Period selection bits (Divider ratio 1:65536; software control of WDTPS)
#pragma config WDTE = OFF       // WDT operating mode (WDT Disabled; SWDTEN is ignored)

// CONFIG3H
#pragma config WDTCWS = WDTCWS_7// WDT Window Select bits (window always open (100%); software control; keyed access not required)
#pragma config WDTCCS = SC      // WDT input clock selector (Software Control)

// CONFIG4L
#pragma config BBSIZE = BBSIZE_512// Boot Block Size selection bits (Boot Block size is 512 words)
#pragma config BBEN = OFF       // Boot Block enable bit (Boot block disabled)
#pragma config SAFEN = OFF      // Storage Area Flash enable bit (SAF disabled)
#pragma config WRTAPP = OFF     // Application Block write protection bit (Application Block not write protected)

// CONFIG4H
#pragma config WRTB = OFF       // Boot Block Write Protection bit (Boot Block not write-protected)
#pragma config WRTC = OFF       // Configuration Register Write Protection bit (Configuration registers not write-protected)
#pragma config WRTD = OFF       // Data EEPROM Write Protection bit (Data EEPROM not write-protected)
#pragma config WRTSAF = OFF     // SAF Write protection bit (SAF not Write Protected)
#pragma config LVP = ON         // Low Voltage Programming Enable bit (Low voltage programming enabled. MCLR/VPP pin function is MCLR. MCLRE configuration bit is ignored)

//CONFIG5L
#pragma config CP = OFF         // PFM and Data EEPROM Code Protection bit (PFM and Data EEPROM code protection disabled)
#include "C:\Program Files\Microchip\xc8\v3.00\pic\include\proc\pic18f47k42.h"

// === CONFIG ===
#define _XTAL_FREQ 4000000   // Oscillator frequency
#define Vref 3.3             // Reference voltage

// === Globals ===
int digital;
float voltage;
float lux;
char data[16];
volatile unsigned char halt_flag = 0;
#define RED_LED LATBbits.LATB0

// === Function Declarations ===
void ADC_Init(void);
void Interrupt_Init(void);

// === ISR ===
void __interrupt(irq(7), base(0x4008)) IOC_ISR(void) {
    if (IOCBFbits.IOCBF1) {
        halt_flag = 1;
        IOCBFbits.IOCBF1 = 0;
    }
}

// === Main ===
void main(void) {
    ADC_Init();
    LCD_Init();
    Interrupt_Init();

    while (1) {
        if (halt_flag) {
            LCD_Clear();
            LCD_String_xy(1, 3, "WAITTTT");  // Centered WAIT message

            for (int i = 0; i < 20; i++) {  // 20 × 500ms = 10 seconds
                RED_LED = 1;
                __delay_ms(250);
                
                RED_LED = 0;
                __delay_ms(250);
             
            }

            halt_flag = 0;
            //LCD_Clear();  // Clear LCD after wait (optional)
        } else {
            ADCON0bits.GO = 1;             // Start ADC conversion
            while (ADCON0bits.GO);         // Wait until done

            digital = (ADRESH << 8) | ADRESL;                    // 10-bit result
            voltage = digital * (Vref / 4096.0);                 // Convert to voltage
            lux = (1.0 - (voltage / Vref)) * 2250.0;             // Estimate lux

            LCD_Clear();
            LCD_String_xy(1, 0, "LDR Reading:");
            sprintf(data, "%.0f lux", lux);
            LCD_String_xy(2, 0, data);

            __delay_ms(800);  // LCD update delay
        }
    }
}

// === ADC Init ===
void ADC_Init(void) {
    TRISAbits.TRISA0 = 1;         // RA0/AN0 input
    ANSELAbits.ANSELA0 = 1;       // Analog enabled

    ADPCH = 0x00;                 // Select AN0
    ADCON0bits.FM = 1;            // Right justified
    ADCON0bits.CS = 1;            // Internal oscillator
    ADCLK = 0x00;
    ADPREL = 0x00;
    ADPREH = 0x00;
    ADACQ = 0x00;
    ADRESL = 0;
    ADRESH = 0;
    ADCON0bits.ON = 1;            // Enable ADC
}

// === Interrupt Init ===
void Interrupt_Init(void) {
    TRISBbits.TRISB1 = 1;         // RB1 as input
    ANSELBbits.ANSELB1 = 0;       // Digital mode
    IOCBPbits.IOCBP1 = 1;         // Interrupt on rising edge
    IOCBNbits.IOCBN1 = 0;
    IOCBFbits.IOCBF1 = 0;         // Clear flag

    PIE0bits.IOCIE = 1;           // Enable IOC interrupt
    PIR0bits.IOCIF = 0;
    INTCON0bits.GIE = 1;          // Global interrupt enable
    INTCON0bits.IPEN = 0;         // Disable priority

    TRISBbits.TRISB0 = 0;         // RB0 as output (RED LED)
    LATBbits.LATB0 = 0;
}
