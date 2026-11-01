/* Modern contiguous USART0 registers at 0xC0..0xC6 (ATmega328P family). */
.macro MINI_UART_INIT
    ldi r16,(MINI_U2X << 1)
    std Y+0,r16
    std Y+1,r18
    ldi r16,0x06
    std Y+2,r16
    ldi r16,(MINI_UBRR & 0xFF)
    std Y+4,r16
#if MINI_UBRR < 256
    std Y+5,r29
#else
    ldi r16,((MINI_UBRR >> 8) & 0x0F)
    std Y+5,r16
#endif
.endm
.macro MINI_UART_STATUS
    ldd r18,Y+0
.endm
.macro MINI_UART_RECEIVE
    ldd r16,Y+6
.endm
.macro MINI_UART_TRANSMIT
    std Y+6,r16
.endm

/* Keep the original single-status-snapshot sequence (UCSR0A is extended I/O). */
.macro MINI_UART_RX_POLL
    ldd r18,Y+0
    sbrs r18,7
    rjmp rx_wait
    sbrs r18,4
    wdr
.endm
.macro MINI_UART_TX_POLL
    ldd r18,Y+0
    sbrs r18,5
    rjmp tx_wait
.endm
