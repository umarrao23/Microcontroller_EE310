//------------------------------------------------------------------
// Title:Keypad-Controlled Access System with Interrupt-based Emergency Stop
//------------------------------------------------------------------
// Purpose  : A two digit password is entered using 4x4 keypad and displayed on lcd.
//            if the correct code is entered a motor is activated.
//            if the code is incorrect, a buzzer is triggered for 10 seconds.
//            An external interrupt (INT0 on RB0) is used to stop the motor
//            and activate the buzzer in emergency situations.
//
//            Special features:
//              -LCD Display
//              -Motor control on RA4
//              -Buzzer alert on RA5
//              -INT0 interrupt (RB0) for emergency press
//
// Compiler :MPLAB X IDE v6.2, XC8 Compiler
// MCU      :PIC18F47K42 
// Author   :Umar Wahid
// Inputs   :Keypad (PORTC for columns(RC4 - RC7),PORTB(RB4 - RB7) for rows,INT0 (RB0)
// Outputs  :LCD (RD0 TO RD7) & (RA0 TO RA2),Motor (RA4),Buzzer (RA5)
// Version  :1.0
//-----------------------------------------------------------------

#include <xc.h>
#include "C:\Program Files\Microchip\xc8\v3.00\pic\include\proc\pic18f47k42.h"

#define _XTAL_FREQ 2000000


// === CONFIGURATION BITS ===
#pragma config FEXTOSC = LP     
#pragma config RSTOSC = EXTOSC  
#pragma config CLKOUTEN = OFF   
#pragma config PR1WAY = ON      
#pragma config CSWEN = ON       
#pragma config FCMEN = ON       
#pragma config MCLRE = EXTMCLR  
#pragma config PWRTS = PWRT_OFF 
#pragma config MVECEN = ON      
#pragma config IVT1WAY = ON     
#pragma config LPBOREN = OFF    
#pragma config BOREN = SBORDIS  
#pragma config BORV = VBOR_2P45 
#pragma config ZCD = OFF        
#pragma config PPS1WAY = ON     
#pragma config STVREN = ON      
#pragma config DEBUG = OFF      
#pragma config XINST = OFF      
#pragma config WDTCPS = WDTCPS_31 
#pragma config WDTE = OFF          
#pragma config WDTCWS = WDTCWS_7  
#pragma config WDTCCS = SC        
#pragma config BBSIZE = BBSIZE_512 
#pragma config BBEN = OFF          
#pragma config SAFEN = OFF         
#pragma config WRTAPP = OFF        
#pragma config WRTB = OFF          
#pragma config WRTC = OFF          
#pragma config WRTD = OFF          
#pragma config WRTSAF = OFF        
#pragma config LVP = ON            
#pragma config CP = OFF

// === LCD Connections ===
#define RS LATAbits.LATA0
#define RW LATAbits.LATA1
#define EN LATAbits.LATA2

// === Keypad Mapping ===
const char keypad_map[4][4] = {
    {'1','2','3','A'},
    {'4','5','6','B'},
    {'7','8','9','C'},
    {'*','0','#','D'}
};

// === LCD Functions ===
void lcd_command(unsigned char cmd) {
    LATD = cmd;
    RS = 0; RW = 0; EN = 1; __delay_ms(1); EN = 0;
    __delay_ms(2);
}

void lcd_data(unsigned char data) {
    LATD = data;
    RS = 1; RW = 0; EN = 1; __delay_ms(1); EN = 0;
    __delay_ms(2);
}

void lcd_string(const char *str) {
    while (*str) lcd_data(*str++);
}

void lcd_init() {
    __delay_ms(20);
    lcd_command(0x38);  // 2-line, 8-bit
    lcd_command(0x0C);  // Display ON, cursor OFF
    lcd_command(0x06);  // Entry mode
    lcd_command(0x01);  // Clear display
    __delay_ms(5);
}

// === Keypad Function ===
char get_key() {
    for (int col = 0; col < 4; col++) {
        LATC = ~(1 << (col + 4)) & 0xF0; // RC4?RC7 columns
        __delay_ms(5);
        if (!PORTBbits.RB4) return keypad_map[0][col];
        if (!PORTBbits.RB5) return keypad_map[1][col];
        if (!PORTBbits.RB6) return keypad_map[2][col];
        if (!PORTBbits.RB7) return keypad_map[3][col];
    }
    return 0;
}

// === Interrupt Initialization ===
void INTERRUPT_Initialize(void) {
    INTCON0bits.IPEN = 1;       // Interrupt priority
    INTCON0bits.GIEH = 1;       // High-priority interrupts
    INTCON0bits.GIEL = 1;       // Low-priority interrupts
    INTCON0bits.INT0EDG = 1;    // Rising edge trigger on RB0
    IPR1bits.INT0IP = 1;        // High priority
    PIR1bits.INT0IF = 0;        // Clear flag
    PIE1bits.INT0IE = 1;        // Enable INT0
}

// === INT0 ISR with debounce ===
void __interrupt(irq(IRQ_INT0), base(0x4008)) INT0_ISR(void) {
    if (PIR1bits.INT0IF) {
        __delay_ms(50); //debounce
        if (PORTBbits.RB0 == 1) {
            LATAbits.LATA4 = 0; //motor OFF
            LATAbits.LATA5 = 1; //buzzer ON
            __delay_ms(10000);  //10 seconds
            LATAbits.LATA5 = 0; //buzzer OFF
            
        }
        PIR1bits.INT0IF = 0; //clear flag
    }
}

// === MAIN ===
void main(void) {
    // === I/O Setup ===
    TRISA = 0x00; ANSELA = 0x00;
    TRISD = 0x00; ANSELD = 0x00;
    TRISC &= 0x0F; ANSELC = 0x00; LATC |= 0xF0;
    TRISB |= 0xF0; ANSELB = 0x00; WPUB = 0xF0;

    TRISAbits.TRISA4 = 0; //motor
    TRISAbits.TRISA5 = 0; //buzzer 
    LATAbits.LATA4 = 0;
    LATAbits.LATA5 = 0;

    TRISBbits.TRISB0 = 1; //RB0 as input
    ANSELBbits.ANSELB0 = 0; //digital input

    INTERRUPT_Initialize();

    lcd_init();
    lcd_string("   Press Key:");

    char key1 = 0, key2 = 0;
    int secret_code = 32;

    while (1) {
        while (!(key1 = get_key()));
        lcd_command(0xC0);
        lcd_data(key1);
        __delay_ms(300);

        while (!(key2 = get_key()));
        lcd_data(key2);
        __delay_ms(300);

        if (key1 >= '0' && key1 <= '9' && key2 >= '0' && key2 <= '9') {
            int entered = (key1 - '0') * 10 + (key2 - '0');
            if (entered == secret_code) {
                PIE1bits.INT0IE = 0; //disable interrupt before motor
                LATAbits.LATA4 = 1;  //motor ON
                lcd_command(0x01);
                lcd_string("    motor");
                __delay_ms(100);
                PIE1bits.INT0IE = 1; //re-enable interrupt
            } else {
                LATAbits.LATA5 = 1; //buzzer ON
                lcd_command(0x01);
                lcd_string(" Wrong Code");
                __delay_ms(10000);
                LATAbits.LATA5 = 0;
            }
        } else {
            lcd_command(0x01);
            lcd_string(" Digits Only");
        }

        __delay_ms(1000);
        lcd_command(0x01);
        lcd_string("Press Key:");
    }
}