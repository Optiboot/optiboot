/* ATtiny1634 USART0: UDR0=0x20, UBRR0L/H=0x21/0x22, UCSR0C/B/A=0x24/0x25/0x26.
 * UCSR0A is OUT-addressable, but outside legal SBIS range; use IN+SBRS.
 */
.macro MINI_UART_INIT
    ldi r16,(MINI_U2X << 1)
    out 0x26,r16
    out 0x25,r18
    ldi r16,0x06
    out 0x24,r16
    ldi r16,(MINI_UBRR & 0xff)
    out 0x21,r16
#if MINI_UBRR < 256
    out 0x22,r29
#else
    ldi r16,((MINI_UBRR >> 8) & 15)
    out 0x22,r16
#endif
.endm
.macro MINI_UART_RX_POLL
    in r18,0x26
    sbrs r18,7
    rjmp rx_wait
    sbrs r18,4
    wdr
.endm
.macro MINI_UART_TX_POLL
    in r18,0x26
    sbrs r18,5
    rjmp tx_wait
.endm
.macro MINI_UART_RECEIVE
    in r16,0x20
.endm
.macro MINI_UART_TRANSMIT
    out 0x20,r16
.endm
