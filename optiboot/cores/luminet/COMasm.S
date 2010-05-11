#include <avr/io.h>

#define	LEFT_TXDDR DDRA
#define	LEFT_TXPORT PORTA
#define LEFT_TXPINNumber  PA2			
		
#define	LEFT_RXDDR  DDRA
#define	LEFT_RXPORT PORTA
#define	LEFT_RXPIN  PINA
#define	LEFT_RXPINNumber PA3			

#define	RIGHT_TXDDR DDRB
#define	RIGHT_TXPORT PORTB
#define RIGHT_TXPINNumber  PB1			
		
#define	RIGHT_RXDDR  DDRB
#define	RIGHT_RXPORT PORTB
#define	RIGHT_RXPIN  PINB
#define	RIGHT_RXPINNumber PB0			
	

#define	UP_TXDDR DDRB
#define	UP_TXPORT PORTB
#define UP_TXPINNumber  PB2			
		
#define	UP_RXDDR  DDRA
#define	UP_RXPORT PORTA
#define	UP_RXPIN  PINA
#define	UP_RXPINNumber PA7

#define	DOWN_TXDDR DDRA
#define	DOWN_TXPORT PORTA
#define DOWN_TXPINNumber  PA1			
		
#define	DOWN_RXDDR  DDRA
#define	DOWN_RXPORT PORTA
#define	DOWN_RXPIN  PINA
#define	DOWN_RXPINNumber PA0			

;***** Global register variables

;bit counter
#define	bitcnt r16

;temporary storage register
#define	temp r17



;***************************************************************************
;*
;* uart_initASM
;*
.global COM_initASM
.func COM_initASM
COM_initASM:		
	sbi _SFR_IO_ADDR(LEFT_TXDDR),LEFT_TXPINNumber
	sbi _SFR_IO_ADDR(LEFT_TXPORT),LEFT_TXPINNumber
	cbi _SFR_IO_ADDR(LEFT_RXDDR),LEFT_RXPINNumber
	
	sbi _SFR_IO_ADDR(RIGHT_TXDDR),RIGHT_TXPINNumber
	sbi _SFR_IO_ADDR(RIGHT_TXPORT),RIGHT_TXPINNumber
	cbi _SFR_IO_ADDR(RIGHT_RXDDR),RIGHT_RXPINNumber
	
	sbi _SFR_IO_ADDR(UP_TXDDR),UP_TXPINNumber
	sbi _SFR_IO_ADDR(UP_TXPORT),UP_TXPINNumber
	cbi _SFR_IO_ADDR(UP_RXDDR),UP_RXPINNumber
	
	sbi _SFR_IO_ADDR(DOWN_TXDDR),DOWN_TXPINNumber
	sbi _SFR_IO_ADDR(DOWN_TXPORT),DOWN_TXPINNumber
	cbi _SFR_IO_ADDR(DOWN_RXDDR),DOWN_RXPINNumber
	
	ret		
.endfunc


;***************************************************************************
;*
;* LEFT_putcASM
;*
.global LEFT_putcASM
.func LEFT_putcASM
LEFT_putcASM:	
	ldi	bitcnt,10	;put 10 bits (start+stop+payload
	com	r24		;Inverte everything
	sec			;Start bit

LEFT_putchar0:	
	brcc	LEFT_putchar1	;If carry set
	cbi	_SFR_IO_ADDR(LEFT_TXPORT),LEFT_TXPINNumber	;    send a '0'
	rjmp	LEFT_putchar2	;else	

LEFT_putchar1:	
	sbi	_SFR_IO_ADDR(LEFT_TXPORT),LEFT_TXPINNumber	;    send a '1'
	nop

LEFT_putchar2:	
	rcall UART_delay	;One bit delay
	rcall UART_delay
	lsr	r24		;Get next bit
	dec	bitcnt		;If not all bit sent
	brne	LEFT_putchar0	;   send next
				;else
	ret			;   return
.endfunc

;***************************************************************************
;*
;* LEFT_getcASM
.global LEFT_getcASM
.func LEFT_getcASM
LEFT_getcASM:	
	ldi 	bitcnt,9	;8 data bit + 1 stop bit

LEFT_getchar1:	
	sbic 	_SFR_IO_ADDR(LEFT_RXPIN),LEFT_RXPINNumber	;Wait for start bit
	rjmp 	LEFT_getchar1
	rcall UART_delay	;0.5 bit delay

LEFT_getchar2:	
	rcall UART_delay	;1 bit delay
	rcall UART_delay		
	clc			;clear carry
	sbic 	_SFR_IO_ADDR(LEFT_RXPIN),LEFT_RXPINNumber	;if RX pin high
	sec			;
	dec 	bitcnt		;If bit is stop bit
	breq 	LEFT_getchar3	;   return
				;else
	ror 	r24		;   shift bit into Rxbyte
	rjmp 	LEFT_getchar2	;   go get next

LEFT_getchar3:	
	ret
.endfunc





;***************************************************************************
;*
;* RIGHT_putcASM
;*
.global RIGHT_putcASM
.func RIGHT_putcASM
RIGHT_putcASM:	
	ldi	bitcnt,10	;put 10 bits (start+stop+payload
	com	r24		;Inverte everything
	sec			;Start bit

RIGHT_putchar0:	
	brcc	RIGHT_putchar1	;If carry set
	cbi	_SFR_IO_ADDR(RIGHT_TXPORT),RIGHT_TXPINNumber	;    send a '0'
	rjmp	RIGHT_putchar2	;else	

RIGHT_putchar1:	
	sbi	_SFR_IO_ADDR(RIGHT_TXPORT),RIGHT_TXPINNumber	;    send a '1'
	nop

RIGHT_putchar2:	
	rcall UART_delay	;One bit delay
	rcall UART_delay
	lsr	r24		;Get next bit
	dec	bitcnt		;If not all bit sent
	brne	RIGHT_putchar0	;   send next
				;else
	ret			;   return
.endfunc

;***************************************************************************
;*
;* RIGHT_getcASM
.global RIGHT_getcASM
.func RIGHT_getcASM
RIGHT_getcASM:	
	ldi 	bitcnt,9	;8 data bit + 1 stop bit

RIGHT_getchar1:	
	sbic 	_SFR_IO_ADDR(RIGHT_RXPIN),RIGHT_RXPINNumber	;Wait for start bit
	rjmp 	RIGHT_getchar1
	rcall UART_delay	;0.5 bit delay

RIGHT_getchar2:	
	rcall UART_delay	;1 bit delay
	rcall UART_delay		
	clc			;clear carry
	sbic 	_SFR_IO_ADDR(RIGHT_RXPIN),RIGHT_RXPINNumber	;if RX pin high
	sec			;
	dec 	bitcnt		;If bit is stop bit
	breq 	RIGHT_getchar3	;   return
				;else
	ror 	r24		;   shift bit into Rxbyte
	rjmp 	RIGHT_getchar2	;   go get next

RIGHT_getchar3:	
	ret
.endfunc





;***************************************************************************
;*
;* UP_putcASM
;*
.global UP_putcASM
.func UP_putcASM
UP_putcASM:	
	ldi	bitcnt,10	;put 10 bits (start+stop+payload
	com	r24		;Inverte everything
	sec			;Start bit

UP_putchar0:	
	brcc	UP_putchar1	;If carry set
	cbi	_SFR_IO_ADDR(UP_TXPORT),UP_TXPINNumber	;    send a '0'
	rjmp	UP_putchar2	;else	

UP_putchar1:	
	sbi	_SFR_IO_ADDR(UP_TXPORT),UP_TXPINNumber	;    send a '1'
	nop

UP_putchar2:	
	rcall UART_delay	;One bit delay
	rcall UART_delay
	lsr	r24		;Get next bit
	dec	bitcnt		;If not all bit sent
	brne	UP_putchar0	;   send next
				;else
	ret			;   return
.endfunc

;***************************************************************************
;*
;* UP_getcASM
.global UP_getcASM
.func UP_getcASM
UP_getcASM:	
	ldi 	bitcnt,9	;8 data bit + 1 stop bit

UP_getchar1:	
	sbic 	_SFR_IO_ADDR(UP_RXPIN),UP_RXPINNumber	;Wait for start bit
	rjmp 	UP_getchar1
	rcall UART_delay	;0.5 bit delay

UP_getchar2:	
	rcall UART_delay	;1 bit delay
	rcall UART_delay		
	clc			;clear carry
	sbic 	_SFR_IO_ADDR(UP_RXPIN),UP_RXPINNumber	;if RX pin high
	sec			;
	dec 	bitcnt		;If bit is stop bit
	breq 	UP_getchar3	;   return
				;else
	ror 	r24		;   shift bit into Rxbyte
	rjmp 	UP_getchar2	;   go get next

UP_getchar3:	
	ret
.endfunc



;***************************************************************************
;*
;* DOWN_putcASM
;*
.global DOWN_putcASM
.func DOWN_putcASM
DOWN_putcASM:	
	ldi	bitcnt,10	;put 10 bits (start+stop+payload
	com	r24		;Inverte everything
	sec			;Start bit

DOWN_putchar0:	
	brcc	DOWN_putchar1	;If carry set
	cbi	_SFR_IO_ADDR(DOWN_TXPORT),DOWN_TXPINNumber	;    send a '0'
	rjmp	DOWN_putchar2	;else	

DOWN_putchar1:	
	sbi	_SFR_IO_ADDR(DOWN_TXPORT),DOWN_TXPINNumber	;    send a '1'
	nop

DOWN_putchar2:	
	rcall UART_delay	;One bit delay
	rcall UART_delay
	lsr	r24		;Get next bit
	dec	bitcnt		;If not all bit sent
	brne	DOWN_putchar0	;   send next
				;else
	ret			;   return
.endfunc

;***************************************************************************
;*
;* DOWN_getc
.global DOWN_getcASM
.func DOWN_getcASM
DOWN_getcASM:	
	ldi 	bitcnt,9	;8 data bit + 1 stop bit

DOWN_getchar1:	
	sbic 	_SFR_IO_ADDR(DOWN_RXPIN),DOWN_RXPINNumber	;Wait for start bit
	rjmp 	DOWN_getchar1
	rcall UART_delay	;0.5 bit delay

DOWN_getchar2:	
	rcall UART_delay	;1 bit delay
	rcall UART_delay		
	clc			;clear carry
	sbic 	_SFR_IO_ADDR(DOWN_RXPIN),DOWN_RXPINNumber	;if RX pin high
	sec			;
	dec 	bitcnt		;If bit is stop bit
	breq 	DOWN_getchar3	;   return
				;else
	ror 	r24		;   shift bit into Rxbyte
	rjmp 	DOWN_getchar2	;   go get next

DOWN_getchar3:	
	ret
.endfunc


;***************************************************************************
;*
;* "UART_delay"
;***************************************************************************
; Some b values: 	(See also table in Appnote documentation)
;
; 1 MHz crystal:
;   9600 bps - b=14
;  19200 bps - b=5
;  28800 bps - b=2
;
; 2 MHz crystal:
;  19200 bps - b=14
;  28800 bps - b=8
;  57600 bps - b=2

; 4 MHz crystal:
;  19200 bps - b=31
;  28800 bps - b=19
;  57600 bps - b=8
; 115200 bps - b=2

;.equ	b	=31	;19200 bps @ 4 MHz crystal
b	=14	;9600 bps @ 1 MHz crystal
;b	=8	;115000 bps @ 8 MHz crystal
	


UART_delay:	
	ldi	temp,b
UART_delay1:	
	dec	temp
	brne	UART_delay1
	ret




