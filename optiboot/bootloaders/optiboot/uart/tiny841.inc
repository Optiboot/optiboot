/* ATtiny841 USART0 at extended memory addresses 0x80..0x86.
 * Experimental Virtual Boot target; not yet tested on hardware.
 */
.macro MINI_UART_INIT
    ldi r16,(MINI_U2X << 1)
    sts 0x86,r16  ; UCSR0A
    sts 0x85,r18  ; UCSR0B (RXEN0|TXEN0)
    ldi r16,0x06
    sts 0x84,r16  ; UCSR0C 8N1
    ldi r16,(MINI_UBRR & 0xff)
    sts 0x81,r16  ; UBRR0L
#if MINI_UBRR < 256
    sts 0x82,r29
#else
    ldi r16,((MINI_UBRR >> 8) & 15)
    sts 0x82,r16
#endif
.endm
.macro MINI_UART_RX_POLL
    lds r18,0x86
    sbrs r18,7
    rjmp rx_wait
    sbrs r18,4
    wdr
.endm
.macro MINI_UART_TX_POLL
    lds r18,0x86
    sbrs r18,5
    rjmp tx_wait
.endm
.macro MINI_UART_RECEIVE
    lds r16,0x80
.endm
.macro MINI_UART_TRANSMIT
    sts 0x80,r16
.endm
