#include <xc.inc>
#include "C:\Users\umar\MPLABXProjects\final_assignment.X\final_assign.inc"    
PSECT absdata,abs,ovrld  
    
;---------------------------------------------------
; Title: Heating & Cooling System Control
;---------------------------------------------------
; Purpose: Compares measured temperature with reference temperature
; and controls the heating and cooling system accordingly.Furthermore, it saves
; the measured and refrence value in decimals by converting them from hex.
; Compiler: MPLAB X IDE, MPASM
; Author: Umar Wahid
; Outputs: PORTD.2 (Cooling), PORTD.1 (Heating)
; Inputs: Keypad (Reference Temp), Sensor (Measured Temp)
; Version:MPLAB X IDE 6.2
;---------------------------------------------------
 
ORG	0x20               ; Program starts at memory location 0x20
;    
;----------------
; PROGRAM INPUTS
;----------------
;The DEFINE directive is used to create macros or symbolic names for values.
;It is more flexible and can be used to define complex expressions or sequences of instructions.
;It is processed by the preprocessor before the assembly begins.

#define  measuredTempInput 	 15; this is the input value
#define  refTempInput 		 -5 ; this is the input value

;---------------------
; Definitions
;---------------------
#define SWITCH    LATD,2  
#define LED0      PORTD,0
#define LED1	  PORTD,1
    
 
;---------------------
; Program Constants
;---------------------
; The EQU (Equals) directive is used to assign a constant value to a symbolic name or label.
; It is simpler and is typically used for straightforward assignments.
;It directly substitutes the defined value into the code during the assembly process.
    
REG10   equ     10h   // in HEX
REG11   equ     11h
REG01   equ     1h

;-----------------------------
; Define Register Locations
;-----------------------------
   
REF_TEMP       EQU  0x20    ; Reference temperature register
MEASURED_TEMP  EQU  0x21    ; Measured temperature register
CONT_REG       EQU  0x22    ; Control register

;DECIMAL_REF_TEMP  EQU  0x60 ; Decimal storage for reference temp
;DECIMAL_MEASURED_TEMP EQU 0x70 ; Decimal storage for measured temp


; ===========================
; Register Definitions
; ===========================
    HUNDREDS1  EQU 0x62    ; First value - Hundreds place
    TENS1      EQU 0x61    ; First value - Tens place
    ONES1      EQU 0x60    ; First value - Ones place
    HEX_VALUE1 EQU 0x63    ; First HEX value register

    HUNDREDS2  EQU 0x72    ; Second value - Hundreds place
    TENS2      EQU 0x71    ; Second value - Tens place
    ONES2      EQU 0x70    ; Second value - Ones place
    HEX_VALUE2 EQU 0x73    ; Second HEX value register

;-----------------------------
; Start Program Execution
;-----------------------------           
       
START:
    GOTO CONVERSION
BACK:
    MOVLW   0x00
    MOVWF   STATUS


    MOVLW   refTempInput	;INPUT THE VALUE OF REFRENCE TEMP
    MOVWF   REF_TEMP		;MOVE VALUE TO REG20
    
    
    MOVLW   measuredTempInput	;INPUT THE VALUE OF MEASURED TEMP FROM 
				;ENVIRONMENT
    MOVWF   MEASURED_TEMP	;MOVE VALUE TO REG21
  

    BTFSS   MEASURED_TEMP,7	;IF MEASURED IS 7 BIT IS 1 IT WILL SKIP NEXT
    
    GOTO    SOLVE
    GOTO    SOLVE_NEG		;IF MEASURED TEMP IS NEGATIVE IT MEANS IT IS 
				;LESS THAN THAN REFRENCE TEMP M<R
    
SOLVE_NEG:			;SOLUTION WHEN MEASURED IS IN NEGATIVE. RELATIVE
				;CANNOT BE NEGATIVE ACCORING TO REQUIREMENT
    MOVLW   0x1
    MOVWF   CONT_REG
    GOTO    TURN_ON_HEATING    
;    BSF	    LED0
;    BCF	    LED1
;    GOTO    START
    

SOLVE:
    MOVF    REF_TEMP,W		;MOVE VALUE OF MEASURED TEMP AT REGISTER 21 TO 
				;WREG
    SUBWF   MEASURED_TEMP,W	;SUBTRACT IT REFRENCE AND RESULT SAVED IN WREG
				;W=MEASURED - REF {MES WILL BE SUBTRACTED FROM REF}
    
    BTFSS   STATUS,2		; IF ZERO FLAG IS NOT SET, SKIP NEXT INSTRUCTION
    
    GOTO    CHECK_LESS	;IF ZERO FLAG IS NOT SET THEN ITS GREATER THAN
				;REALATIVE
    GOTO    EQUAL_TEMP

EQUAL_TEMP:

    BCF	    LED0	    ;TURN OF HEATING [BCF IS BIT CLEAR] LEDO=PORTD,0
    BCF	    LED1	    ;TURN OF COOLING			LED1=PORT,0
    MOVLW   0X00
    MOVWF   CONT_REG	    ;CONT+REG=0
    GOTO    START

CHECK_LESS:	    ;IF MEASURED IS LESS THAN REFRENCE
    BTFSS   STATUS,4
    GOTO    CHECK_GREATER
    GOTO    TURN_ON_HEATING
    
    
    
CHECK_GREATER:	    ;IF MEASURE IS GREATER THAN REFRENCE
    MOVLW   0X02
    MOVWF   CONT_REG
    GOTO    TURN_ON_COOLING
    
    
TURN_ON_COOLING:	;IF REFRENCE IS LESS THEN MEASURED IT WILL COME HERE
    MOVLW   0XFA
    MOVWF   TRISD
    MOVLW   0X01    
    MOVWF   CONT_REG
    MOVF    CONT_REG,W
    MOVWF   PORTD
    GOTO    START	;CHECK TEMPERTURE AGAIN

TURN_ON_HEATING:	;IF REFRENCE IS GREATER THAN MEASURE IT WILL COME HERE
    MOVLW   0XFA
    MOVWF   TRISD
    MOVLW   0X02    
    MOVWF   CONT_REG
    MOVF    CONT_REG,W
    MOVWF   PORTD
    GOTO    START
    
    
    
    
    
    
    
    
    
    
    
    
    
CONVERSION:
     ;FIRST VALUE
    CLRF    HUNDREDS1	    ;clear it 
    CLRF    TENS1	    ;clear it
    CLRF    ONES1	    ;clear it
     
    MOVLW   refTempInput    ;decimal value
    MOVWF   HEX_VALUE1	    ;save to this place
    
    CALL    CONVERT_HEX1    ;function is called
    
    ;SECOND VALUE
    CLRF    HUNDREDS2	    ;clear it	
    CLRF    TENS2	;clear it
    CLRF    ONES2	;clear it
    
    MOVLW   measuredTempInput 		;decimal value
    MOVWF   HEX_VALUE2	    ;hex value 2
    CALL    CONVERT_HEX2    ;gunction 2 is called
    GOTO    BACK
    
    CONVERT_HEX1:
HUNDRED_LOOP_1:
    MOVLW   100		;100 to wreg
    CPFSGT  HEX_VALUE1	;COMPARE hex WITH W.SKIP NEXT INSTRUCTION IF hex>W.IT CHECKS 
			;IF 100 IS GREATER THAN HEX_VALUE WHICH IS 234
    GOTO    TENS_LOOP_1	    ; will take to tens loop
    SUBWF   HEX_VALUE1,F	    ;HEX_VALUE1-WREG=HEX_VALUE1 saved in same reg
    INCF    HUNDREDS1,F	    ;REGISTER HUNDREDS1 GETS INCREMENTED BY ONE save in same reg
    GOTO    HUNDRED_LOOP_1  ;go back to loop again
    
 TENS_LOOP_1:		; to check tens . same process repeated
    MOVLW   10
    CPFSGT  HEX_VALUE1
    GOTO    ONES_LOOP_1
    SUBWF   HEX_VALUE1,F
    INCF    TENS1,F
    GOTO    TENS_LOOP_1

ONES_LOOP_1:		; to check ones . same process repeated
    MOVF    HEX_VALUE1,W
    MOVWF   ONES1
    RETURN
    
CONVERT_HEX2:		; for second number
HUNDRED_LOOP_2:		;; to check hundreds . same process repeated
    MOVLW   100
    CPFSGT  HEX_VALUE2	;COMPARE F WITH W.SKIP NEXT INSTRUCTION IF F>W.IT CHECKS 
			;IF 100 IS GREATER THAN HEX_VALUE WHICH IS 234
    
    GOTO    TENS_LOOP_2
    SUBWF   HEX_VALUE2,F	    ;HEX_VALUE1-WREG=HEX_VALUE1
    INCF    HUNDREDS2,F	    ;REGISTER HUNDREDS1 GETS INCREMENTED BY ONE
    GOTO    HUNDRED_LOOP_2
    
 TENS_LOOP_2:		;; to check tens . same process repeated
    MOVLW   10
    CPFSGT  HEX_VALUE2
    GOTO    ONES_LOOP_2
    SUBWF   HEX_VALUE2,F
    INCF    TENS2,F
    GOTO    TENS_LOOP_2
    
ONES_LOOP_2:		; to check ones . same process repeated
    MOVF    HEX_VALUE2,W
    MOVWF   ONES2
    RETURN