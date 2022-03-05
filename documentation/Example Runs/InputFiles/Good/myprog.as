; Write your own assembly language source program.
; The program should contain at least 10 insructions, 5 labels, 5 directives
; No errors: the assembler should generate successfully all output files:
;     myprog.ob, myprog.ext, myprog.ent
MAIN: sub #5, r3
 mov r3, *r6
 bne END
 lea STR, r1
 inc r3
 add r3, r1
 clr YVARIABLE
 dec r3
 rts
 
HALT: inc r5
 jmp END
 
.extern STR
END: stop
LISTPOSITIVE: .data 1, 2
 .data 3, 4, 5
LISTNEGATIVE: .data -1, -2
 .data -3, -4, -5
XVARIABLE: .data -1
YVARIABLE: .data 0
TEXT: .string "text"
