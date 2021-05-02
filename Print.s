; Print.s
; Student names: Raffer Li Gerald Liu
; Last modification date: change this to the last modification date or look very silly
; Runs on TM4C123
; EE319K lab 7 device driver for any LCD
;
; As part of Lab 7, students need to implement these LCD_OutDec and LCD_OutFix
; This driver assumes two low-level LCD functions
; SSD1306_OutChar   outputs a single 8-bit ASCII character
; SSD1306_OutString outputs a null-terminated string 

    IMPORT   SSD1306_OutChar
    IMPORT   SSD1306_OutString
    EXPORT   LCD_OutDec
    EXPORT   LCD_OutFix
    PRESERVE8
    AREA    |.text|, CODE, READONLY, ALIGN=2
    THUMB



;-----------------------LCD_OutDec-----------------------
; Output a 32-bit number in unsigned decimal format
; Input: R0 (call by value) 32-bit unsigned number
; Output: none
; Invariables: This function must not permanently modify registers R4 to R11
N    EQU    0
CNT 	   EQU    4
LCD_OutDec
    PUSH{R4-R11}
    SUBS SP, #8                ; allocate space
    MOV R4, SP

    MOV R5, #10
    MOV R6, #0                ; counter
    STR R6, [R4, #CNT]
    STR R0, [R4, #N]        ; store original number into CNT

stackloop
    LDR R7, [R4, #N]        ; load current CNT
    UDIV R8, R7, R5            ; R8 = CNT/10
    STR R8, [R4, #N]        ; store new CNT
    MUL R8, R8, R5            ; R8 =     R8*10
    SUBS R9, R7, R8            ; R9 = CNT - R8
    PUSH{R9, R10}            ; push R9 onto stack
    LDR R6, [R4, #CNT]
    ADD R6, R6, #1
    STR R6, [R4, #CNT]
    LDR R7, [R4, #N]
    CMP R7, #0
    BNE stackloop

printerloop
    POP{R9, R10}
    ADD R0, R9, #0x30
    PUSH{LR, R10}
    BL    SSD1306_OutChar
    POP{LR, R10}
    LDR R6, [R4, #CNT]
    SUB R6, R6, #1
    STR R6, [R4, #CNT]
    CMP R6, #0
    BNE printerloop

    ADD    SP, #8                ; deallocate space
    POP {R4-R11}
    BX LR
;* * * * * * * * End of LCD_OutDec * * * * * * * *

; -----------------------LCD _OutFix----------------------
; Output characters to LCD display in fixed-point format
; unsigned decimal, resolution 0.01, range 0.00 to 9.99
; Inputs:  R0 is an unsigned 32-bit number
; Outputs: none
; E.g., R0=0,    then output "0.00 "
;       R0=3,    then output "0.03 "
;       R0=89,   then output "0.89 "
;       R0=123,  then output "1.23 "
;       R0=999,  then output "9.99 "
;       R0>999,  then output "*.** "
; Invariables: This function must not permanently modify registers R4 to R11
hundredth EQU 0
tenth EQU 4
ones EQU 8
asterisk EQU 0x2A
dot EQU 0x2E
	
LCD_OutFix
	PUSH{R4-R11}	
	MOV R1,#0
	MOV R2,#0
	MOV R3,#0
	MOV R4,#100
	MOV R5,#10
	
	SUB SP, #16 ;allocating space
	CMP R0,#1000
	BGE outRange
	CMP R0, #0
	BLT outRange
	
	UDIV R3,R0,R4
	STR R3,[SP,#ones]
subtractOne
	CMP R3,#0
	BEQ divide10
	SUB R0,R0,R4
	SUB R3,R3,#1
	B subtractOne
divide10
	UDIV R2,R0,R5
	STR R2,[SP,#tenth]
subtract10
	CMP R2,#0
	BEQ divide100
	SUB R0,R0,R5
	SUB R2,R2,#1
	B subtract10
divide100
	STR R0,[SP,#hundredth]
	
	;begin print
	LDRB R0,[SP,#ones]	
	ADD R0,#0x30
	PUSH {LR, R6}
	BL SSD1306_OutChar
	POP {LR, R6}
	MOV R0,#dot
	PUSH {LR, R6}
	BL SSD1306_OutChar
	POP {LR, R6}
	LDRB R0,[SP,#tenth]
	ADD R0,#0x30
	PUSH {LR, R6}
	BL SSD1306_OutChar
	POP {LR, R6}
	LDRB R0,[SP,#hundredth]
	ADD R0,#0x30
	PUSH {LR, R6}
	BL SSD1306_OutChar
	POP {LR, R6}
	B returnBack
	
outRange	
	MOV R0,#asterisk
	PUSH {LR, R6}
	BL SSD1306_OutChar
	POP {LR, R6}
	MOV R0,#dot
	PUSH {LR, R6}
	BL SSD1306_OutChar
	POP {LR, R6}
	MOV R0,#asterisk
	PUSH {LR, R6}
	BL SSD1306_OutChar
	POP {LR, R6}
	MOV R0,#asterisk
	PUSH {LR, R6}
	BL SSD1306_OutChar
	POP {LR, R6}
	
returnBack
	ADD SP,#16			;deallocate
    POP {R4 - R11}
    BX LR
 
     ALIGN
;* * * * * * * * End of LCD_OutFix * * * * * * * *

     ALIGN          ; make sure the end of this section is aligned
     END            ; end of file
