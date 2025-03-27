
#include <xc.inc>
#include "C:Users\umar\MPLABXProjects\Counter.X\file.inc"
PSECT absdata,abs,ovrld	
;--------------------------------------------------------------
;  Title:     Counter Using A Keypad
;--------------------------------------------------------------
;  Purpose: Increments and decrements 7 segement display from
;  from values ranging from 0 to F in hex. Furthermore, keypad
;  is used to change the values on 7 segemnt. When 1 is pressed
;  increment is done when 2 is pressed decrement happens and when
;  both are pressed the 7 segement reset to 0.
;  Compiler:  MPLAB X IDE v6.20 (MPASM/XC8 Assembly)
;  Author:  Umar Wahid
;  Outpus:  R0 To RD7 for 7 segment
;	    RB4 for keypad
;  Inputs:  RB3 and RB0 for keypad
;  Version: MPLAB X IDE 6.2
;--------------------------------------------------------------



;--------------------------------------------------------------
;  CONSTANTS & EQU
;--------------------------------------------------------------
Inner_loop  equ 255     ; For delay routine (inner count)
Outer_loop  equ 255    ; For delay routine (outer count)

;--------------------------------------------------------------
;   Register locations
;--------------------------------------------------------------
  
REG10       equ 0x10     ; Temporary registers for delays
REG11       equ 0x11
      
REG20	    equ	0x20
REG21	    equ	0x21
REG22	    equ	0x22
REG23	    equ	0x23
REG24	    equ 0x24
REG25	    equ 0x25
REG26	    equ	0x26
REG27	    equ 0x27
REG28	    equ	0x28
REG29	    equ	0x29
REG0A	    equ 0x2A
REG0B	    equ 0x2B
REG0C	    equ	0x2C
REG0D	    equ 0x2D
REG0E	    equ	0x2E
REG0F	    equ 0x2F
  
;--------------------------------------------------------------
;  MEMORY SECTION
;--------------------------------------------------------------

ORG 0x0000                   ; Reset vector at address 0
GOTO _setup              ; Jump to 
ORG 0x0050               ; We'll place main code starting here
   
;--------------------------------------------------------------
;  SETUP & MAIN PROGRAM
;--------------------------------------------------------------
_setup:

    CALL _setupPortA     ; Configure PORTD for 7-segment output
    CALL _setupPortB     ; Configure PORTB for keypad
    CALL POINTER
    CALL TABLE
    CALL WAIT_FOR_KEY1
   
    
_setupPortA:
;---------------------------------------------------------
;  SETUP PORT D FOR 7 SEGMENT 
;---------------------------------------------------------  
    BANKSEL PORTD
    CLRF    PORTD
    BANKSEL LATD
    CLRF    LATD
    BANKSEL ANSELD
    CLRF    ANSELD            
    BANKSEL TRISD	
    CLRF    TRISD   ;PORT D IS OUTPUT
    Return

_setupPortB:
;---------------------------------------------------------
; PORT B SETUP
;---------------------------------------------------------  
    BANKSEL ANSELB
    CLRF    ANSELB          ; RB0 and RB4 AND RB3 digital only Analod disable

    BANKSEL TRISB
    BCF     TRISB, 4        ; RB4 = output  ;configure rb4 output (0)
        
    BSF     TRISB, 0	    ; RB0 = input(1)
    BSF	    TRISB, 3	    ;RB3 AS INPUT
    
    ;external pull up on rbo and rb3
    BANKSEL WPUB
    BSF     WPUB, 0         ; Enable pull-up on RB0
    BSF	   WPUB, 3
    
    ;pull colmn 1 low
    BANKSEL LATB
    BCF     LATB, 4         ; RB4 = 0  
   
   RETURN
  
POINTER:
    MOVLW   0X20
    MOVWF   FSR0L
    CLRF    FSR0H
    
    MOVLW   0X3F
    MOVWF   LATD
    

TABLE:
    MOVLW   0x3F    ;0
    MOVWF   REG20 
    
    MOVLW   0x30    ;1
    MOVWF   REG21   
    
    MOVLW   0x5B    ;2
    MOVWF   REG22
    
    MOVLW   0x4F    ;3
    MOVWF   REG23
    
    MOVLW   0X66    ;4
    MOVWF   REG24
    
    MOVLW   0X6D    ;5
    MOVWF   REG25
    
    MOVLW   0X7D    ;6
    MOVWF   REG26
    
    MOVLW   0X07   ;7
    MOVWF   REG27
    
    MOVLW   0X7F    ;8
    MOVWF   REG28
    
    MOVLW   0X67    ;9
    MOVWF   REG29
    
    MOVLW   0X77    ;A
    MOVWF   REG0A
    
    MOVLW   0X7C    ;B
    MOVWF   REG0B
    
    MOVLW   0X39    ;C
    MOVWF   REG0C
    
    MOVLW   0X5E    ;D
    MOVWF   REG0D
    
    MOVLW   0X79    ;E
    MOVWF   REG0E
    
    MOVLW   0X71    ;F
    MOVWF   REG0F
    
   
WAIT_FOR_KEY1:
    BANKSEL PORTB
    MOVF    PORTB, W
    ANDLW   0x09            ;check RB0 & RB3
    BZ      HANDLE_RESET    ;both LOW ? do reset

    ;if only RB0 is pressed
    BANKSEL PORTB
    BTFSS   PORTB, 0        ;if RB0 is LOW (pressed)
    GOTO    LOOP            ;go to increment

    ;if only RB3 is pressed
    BANKSEL PORTB
    BTFSS   PORTB, 3        ;if RB3 is LOW (pressed)
    GOTO    LOOP2           ;go to decrement

    GOTO    WAIT_FOR_KEY1   ;none pressed ? keep waiting


HANDLE_RESET:	; if both are pressed
    MOVLW   0X20    
    MOVWF   FSR0L   ;move fsr0l to 0x20 
    MOVF    INDF0,W ;move that value to fsr0l
    MOVWF   LATD    ;move to the value to latch d
    CALL    DELAY   ;call delay
    GOTO    WAIT_FOR_KEY1   
    
    
LOOP2:	 ;FOR BUTTON 2 DECREMENT
    MOVLW   0X00    ;MOVE ZERO TO STATUS
    MOVWF   STATUS  ;CLEAR STATUS
    MOVLW   0X20    ;
    SUBWF   FSR0L,W  ; TO CHECK IF ITS AT LOCATION 20
    BZ	    HANDLE_ZERO
    
    DECF    FSR0L,F ;DECREMNT
    MOVF    INDF0,W ;MOVE VALUE IN FSR TO W
    MOVWF   LATD    
    CALL    DELAY
    GOTO    WAIT_FOR_KEY1

HANDLE_ZERO:	;HANDLING DECREMRNT
    MOVLW   0X30
    MOVWF   FSR0L
    GOTO    LOOP2
    

LOOP:	;FOR BUTTON 1
    MOVLW   0X00    ;MOVE ZERO TO STATUS
    MOVWF   STATUS  ;CLEAR STATUS
    MOVLW   0X2F    ;
    SUBWF   FSR0L,W  ; TO CHECK IF ITS AT LOCATION 2F
    BZ	    HANDLE_2F
    
    
    INCF    FSR0L,F	;INCREMENT FSR
    MOVF    INDF0,W	;VALUE GOES TO WORKING
    MOVWF   LATD
    ;DECFSZ  COUNT,F	;COUNT
    CALL    DELAY
    GOTO    WAIT_FOR_KEY1

HANDLE_2F:  ;HANDLE INCREMENT
    MOVLW   0X1F
    MOVWF   FSR0L   ;if value exceeds 2F
    GOTO    LOOP
     
DELAY:	;to call the delay
    MOVLW   Inner_loop 
    MOVWF   REG10   
    
    MOVLW   Outer_loop
    MOVWF   REG11

_loop1:
    DECF    REG10,1
    BNZ	    _loop1
    
    NOP
    NOP
    NOP
    NOP
    
    MOVLW   Inner_loop
    MOVWF   REG10
    
    DECF    REG11,1
    BNZ	    _loop1
    
    RETURN
