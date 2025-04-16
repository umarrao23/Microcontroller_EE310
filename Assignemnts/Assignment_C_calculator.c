//------------------------------------------------------------------------------
// Title    : 2-Digit Calculator with Error Detection and Negative Display
//------------------------------------------------------------------------------
// Purpose  : Implements a basic calculator on the PIC18F47K42 using a 4x4 keypad
//            and two 7-segment displays. Supports addition, subtraction,
//            multiplication, and division of two 2-digit numbers.
//
//            Special Features:
//              - Displays EE on overflow or when divide by zero
//              - DOT segment (RD7) lights up for negative results
//              - User inputs:2-digit number, operator, 2-digit number
//              - Result range:99 to +99
//              - '#' confirms operation
//              - '*' resets system
//
// Compiler : MPLAB X IDE v6.2, XC8 Compiler
// MCU      : PIC18F47K42 
// Author   : Umar Wahid
// Date     : April 2025
// Inputs   : Keypad (PORTB - 4x4 matrix)
// Outputs  : 7-Segment Display (PORTA & PORTD)
// Version  : 1.0
//------------------------------------------------------------------------------



// CONFIG1L

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

// CONFIG5L
#pragma config CP = OFF         // PFM and Data EEPROM Code Protection bit (PFM and Data EEPROM code protection disabled)

#include <xc.h>
#include "C:\Program Files\Microchip\xc8\v3.00\pic\include\proc\pic18f47k42.h"

#define _XTAL_FREQ 2000000  // 2 MHz clock for delay


// 7-Segment lookup table (Common Cathode)
const unsigned char segment[16] = {
    0x3F, // 0
    0x06, // 1
    0x5B, // 2
    0x4F, // 3
    0x66, // 4
    0x6D, // 5
    0x7D, // 6
    0x07, // 7
    0x7F, // 8
    0x67  // 9
};

// Keypad mapping
const char keypad_map[4][4] = {
    { 1, 2, 3, 16 },   // A = Add
    { 4, 5, 6, 17 },   // B = Sub
    { 7, 8, 9, 18 },   // C = Multiply
    {13, 0,15, 19 }    // * = 13, # = 15, D = Divide
};

// Global variables
unsigned char key;
unsigned char x_input_reg, y_input_reg;
unsigned char x_high = 0, x_low = 0, y_high = 0, y_low = 0;

int check_keypad();

void main() {
    // 7-segment setup
    TRISA = 0x00; ANSELA = 0x00; LATA = 0x00;
    TRISD = 0x00; ANSELD = 0x00; LATD = 0x00;

    // Keypad setup
    TRISB = 0xF0; ANSELB = 0x00; WPUB = 0xF0;
    TRISC = 0x00; ANSELC = 0x00;

    int count = 0;

    while (1) {
        key = check_keypad();

        if (count == 0 && key >= 0 && key <= 9) {
            x_high = key;
            PORTA = segment[x_high];
            LATDbits.LATD7 = 0;
            count = 1;
            __delay_ms(250);
        }

        else if (count == 1 && key >= 0 && key <= 9) {
            x_low = key;
            PORTD = segment[x_low];
            count = 2;
            __delay_ms(250);
        }

        // === ADDITION ===
        else if (count == 2 && key == 16) {
            x_input_reg = x_high * 10 + x_low;
            PORTA = PORTD = 0x00; count = 0;

            while (1) {
                key = check_keypad();
                if (count == 0 && key >= 0 && key <= 9) {
                    y_high = key; PORTA = segment[y_high]; count = 1; __delay_ms(250);
                }
                else if (count == 1 && key >= 0 && key <= 9) {
                    y_low = key; PORTD = segment[y_low]; count = 2; __delay_ms(250);
                }
                else if (count == 2 && key == 15) {
                    y_input_reg = y_high * 10 + y_low;
                    int result = x_input_reg + y_input_reg;
                    if (result > 99 || result < -99) {
                        PORTA = PORTD = 0x79; LATDbits.LATD7 = 0; break;
                    }
                    unsigned char absResult = (result < 0) ? -result : result;
                    PORTA = segment[absResult / 10];
                    PORTD = segment[absResult % 10];
                    LATDbits.LATD7 = (result < 0) ? 1 : 0;
                    break;
                }
            }
        }

        // === SUBTRACTION ===
        else if (count == 2 && key == 17) {
            x_input_reg = x_high * 10 + x_low;
            PORTA = PORTD = 0x00; count = 0;

            while (1) {
                key = check_keypad();
                if (count == 0 && key >= 0 && key <= 9) {
                    y_high = key; PORTA = segment[y_high]; count = 1; __delay_ms(250);
                }
                else if (count == 1 && key >= 0 && key <= 9) {
                    y_low = key; PORTD = segment[y_low]; count = 2; __delay_ms(250);
                }
                else if (count == 2 && key == 15) {
                    y_input_reg = y_high * 10 + y_low;
                    int result = x_input_reg - y_input_reg;
                    if (result > 99 || result < -99) {
                        PORTA = PORTD = 0x79; LATDbits.LATD7 = 0; break;
                    }
                    unsigned char absResult = (result < 0) ? -result : result;
                    PORTA = segment[absResult / 10];
                    PORTD = segment[absResult % 10];
                    LATDbits.LATD7 = (result < 0) ? 1 : 0;
                    break;
                }
            }
        }

        // === MULTIPLICATION ===
        else if (count == 2 && key == 18) {
            x_input_reg = x_high * 10 + x_low;
            PORTA = PORTD = 0x00; count = 0;

            while (1) {
                key = check_keypad();
                if (count == 0 && key >= 0 && key <= 9) {
                    y_high = key; PORTA = segment[y_high]; count = 1; __delay_ms(250);
                }
                else if (count == 1 && key >= 0 && key <= 9) {
                    y_low = key; PORTD = segment[y_low]; count = 2; __delay_ms(250);
                }
                else if (count == 2 && key == 15) {
                    y_input_reg = y_high * 10 + y_low;
                    int result = x_input_reg * y_input_reg;
                    if (result > 99 || result < -99) {
                        PORTA = PORTD = 0x79; LATDbits.LATD7 = 0; break;
                    }
                    unsigned char absResult = (result < 0) ? -result : result;
                    PORTA = segment[absResult / 10];
                    PORTD = segment[absResult % 10];
                    LATDbits.LATD7 = (result < 0) ? 1 : 0;
                    break;
                }
            }
        }

        // === DIVISION ===
        else if (count == 2 && key == 19) {
            x_input_reg = x_high * 10 + x_low;
            PORTA = PORTD = 0x00; count = 0;

            while (1) {
                key = check_keypad();
                if (count == 0 && key >= 0 && key <= 9) {
                    y_high = key; PORTA = segment[y_high]; count = 1; __delay_ms(250);
                }
                else if (count == 1 && key >= 0 && key <= 9) {
                    y_low = key; PORTD = segment[y_low]; count = 2; __delay_ms(250);
                }
                else if (count == 2 && key == 15) {
                    y_input_reg = y_high * 10 + y_low;
                    if (y_input_reg == 0) {
                        PORTA = PORTD = 0x79; LATDbits.LATD7 = 0; break;
                    }
                    int result = x_input_reg / y_input_reg;
                    if (result > 99 || result < -99) {
                        PORTA = PORTD = 0x79; LATDbits.LATD7 = 0; break;
                    }
                    unsigned char absResult = (result < 0) ? -result : result;
                    PORTA = segment[absResult / 10];
                    PORTD = segment[absResult % 10];
                    LATDbits.LATD7 = (result < 0) ? 1 : 0;
                    break;
                }
            }
        }

        // === RESET ===
        else if (key == 13) {
            PORTA = PORTD = 0x00;
            LATDbits.LATD7 = 0;
            count = 0;
            __delay_ms(250);
        }
    }
}

// === KEYPAD SCAN FUNCTION ===
int check_keypad() {
    for (int col = 0; col < 4; col++) {
        LATB = ~(1 << col) & 0x0F;
        __delay_ms(250);
        if (PORTBbits.RB4 == 0) return keypad_map[0][col];
        if (PORTBbits.RB5 == 0) return keypad_map[1][col];
        if (PORTBbits.RB6 == 0) return keypad_map[2][col];
        if (PORTBbits.RB7 == 0) return keypad_map[3][col];
    }
    return -1;
}



