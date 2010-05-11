#include <avr/io.h>

#define	temp r17
#define	temptwo r16
#define pin r24
#define value r22

;***************************************************************************
;*
;* digitalWriteASM
;* r24 is pin
;* r22 is value
;*
.global digitalWriteASM
.func digitalWriteASM
digitalWriteASM:
	ldi temptwo,0
calcpin:
	cpi pin,0
	brne notPinZero
	ldi pin,11
notPinZero:
	cpi pin,4
	brlt isPortB
isPortA:
	subi pin,4
	rjmp endcalcpin
isPortB:
	ldi temptwo,1
	ldi temp,3
	sub temp,pin
	mov pin,temp
endcalcpin:
	nop
calcmask:
	ldi temp,1
	cpi pin,0
	breq endcalcmask
shiftLeft:
	lsl temp
	dec pin
	brne shiftLeft
endcalcmask:
	cpi temptwo,1
	brne doitForA
doitForB:
	in temptwo, PORTB
	cpi value,1
	brne doandb
	OR temptwo, temp
	rjmp setb
doandb:
	com temp
	AND temptwo,temp
setb:
	sts PORTB, temptwo
	rjmp enddigitalwrite
doitForA:
	in temptwo, PORTA
	cpi value,1
	brne doanda
	OR temptwo, temp
	rjmp seta
doanda:
	com temp
	AND temptwo,temp
seta:
	sts PORTA, temptwo
enddigitalwrite:
	ret
.endfunc

