/* ATmega64A/ATmega128A USART0 registers: mixed I/O and extended data addresses.
 * Do NOT use the ATmega16 UCSRC/UBRRH URSEL rule here.
 */
.macro MINI_UART_INIT
    ldi r16,(MINI_U2X << 1)
    out UART_A_IO,r16
    out UART_B_IO,r18
    ldi r16,0x06
    sts UART_C_MEM,r16
    ldi r16,(MINI_UBRR & 0xFF)
    out UART_L_IO,r16
    ldi r16,((MINI_UBRR >> 8) & 0x0F)
    sts UART_H_MEM,r16
.endm
.macro MINI_UART_STATUS
    in r18,UART_A_IO
.endm
.macro MINI_UART_RECEIVE
    in r16,UART_DATA_IO
.endm
.macro MINI_UART_TRANSMIT
    out UART_DATA_IO,r16
.endm

/* Two direct status tests are safe here: USART frame status remains
 * associated with the unread character until UDR is read. Interrupts are off.
 * Both tests use 0..31 I/O addresses, the legal range for SBIS.
 */
.macro MINI_UART_RX_POLL
    sbis UART_A_IO,7
    rjmp rx_wait
    sbis UART_A_IO,4
    wdr
.endm
.macro MINI_UART_TX_POLL
    sbis UART_A_IO,5
    rjmp tx_wait
.endm
