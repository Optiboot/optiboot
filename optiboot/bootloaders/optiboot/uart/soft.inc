/* Software UART 8N1, no timer/interrupt use. ATtiny84: PA1 TX PA2 RX;
 * ATtiny85: PB0 TX PB1 RX. RX idle=1, start=0, LSB first, stop=1.
 *
 * Timing assumes F_CPU is the ACTUAL CPU frequency (CLKDIV8 disabled
 * when 8 MHz RC is selected). Bit time is rounded to nearest cycle.
 * Registers r6 (shift) and r18 (delay) dedicated to software UART;
 * r16 provides transmit/receive value. No interrupts (boot starts with CLI).
 * Timing diagrams/tests are in mini_tests/test_soft_uart.py.
 * Based on original code; license GPL-2.0-or-later.
 */
#define SOFT_BIT_CYCLES ((F_CPU + BAUD_RATE / 2) / BAUD_RATE)
#if SOFT_BIT_CYCLES < 100 || SOFT_BIT_CYCLES > 750
#error Unsupported software UART baud/F_CPU: need 100..750 CPU cycles per bit
#endif
/* Loop body: DEC 1 + BRNE taken 2, final 1; LDI 1, RET 4,
 * RCALL 3 => 3*N+7 cycles in total.
 * Rx start sample: choose 3*N+9 approximately half-bit.
 * Rx data: choose 3*N+12 per bit (sampling and loop overhead).
 * TX between pin edges: 3*N+13..15 cycles including branches.
 */
#define SOFT_HALF_N ((SOFT_BIT_CYCLES / 2 - 9 + 1) / 3)
#define SOFT_RX_N ((SOFT_BIT_CYCLES - 12 + 1) / 3)
#define SOFT_TX_N ((SOFT_BIT_CYCLES - 14 + 1) / 3)
#define SOFT_TX_START_N ((SOFT_BIT_CYCLES - 10 + 1) / 3)
#if SOFT_HALF_N < 1 || SOFT_HALF_N > 255 || SOFT_RX_N < 1 || SOFT_RX_N > 255 || SOFT_TX_N < 1 || SOFT_TX_N > 255
#error Software UART delay-counter overflow
#endif

.macro MINI_UART_INIT
    ; Set output latch high BEFORE switching DDR to output: no false start.
    sbi SOFT_PORT_IO,SOFT_TX_BIT
    cbi SOFT_DDR_IO,SOFT_RX_BIT
    sbi SOFT_PORT_IO,SOFT_RX_BIT  ; optional pullup for idle RX
    sbi SOFT_DDR_IO,SOFT_TX_BIT
.endm
.macro MINI_UART_RX_POLL
.endm
.macro MINI_UART_RECEIVE
    rcall soft_uart_rx
.endm
.macro MINI_UART_TX_POLL
.endm
.macro MINI_UART_TRANSMIT
    rcall soft_uart_tx
.endm

.macro MINI_UART_SOFT_ROUTINES
/* UART receive, blocking. Verifies start at 0.5 bit and stop at 9.5.
 * Polling WDR while idle prevents the 1s watchdog reset. Malformed stop
 * bits are discarded and polling continues. Unlike the hardware UART
 * path, this receiver does not provide a watchdog inactivity timeout.
 */
soft_uart_rx:
.Lsoft_rx_wait_start:
    wdr
    sbic SOFT_PIN_IO, SOFT_RX_BIT
    rjmp .Lsoft_rx_wait_start
    rcall .Lsoft_half_delay
    sbic SOFT_PIN_IO, SOFT_RX_BIT
    rjmp .Lsoft_rx_wait_start    ; glitch: line high at mid-start
    rcall .Lsoft_rx_delay
    ldi r16, 8
    clr r6
.Lsoft_rx_data:
    clc
    sbic SOFT_PIN_IO, SOFT_RX_BIT
    sec
    ror r6
    rcall .Lsoft_rx_delay
    dec r16
    brne .Lsoft_rx_data
    sbis SOFT_PIN_IO, SOFT_RX_BIT
    rjmp .Lsoft_rx_wait_start    ; framing error: discard incomplete byte
    mov r16,r6
    ret

.Lsoft_half_delay:
    ldi r18, SOFT_HALF_N
.Lsoft_half_loop:
    dec r18
    brne .Lsoft_half_loop
    ret
.Lsoft_rx_delay:
    ldi r18, SOFT_RX_N
.Lsoft_rx_loop:
    dec r18
    brne .Lsoft_rx_loop
    ret

/* Transmit: no interrupts, 1 start + 8 data + 1 stop, LSB first.
 * Branch-balanced pin action (set/clear paths both five cycles), so
 * baud is almost independent of the bit pattern.
 */
soft_uart_tx:
    mov r6,r16
    ldi r16,8
    cbi SOFT_PORT_IO,SOFT_TX_BIT
    rcall .Lsoft_tx_start_delay
.Lsoft_tx_data:
    sbrc r6,0
    sbi SOFT_PORT_IO,SOFT_TX_BIT
    sbrs r6,0
    cbi SOFT_PORT_IO,SOFT_TX_BIT
    lsr r6
    rcall .Lsoft_tx_delay
    dec r16
    brne .Lsoft_tx_data
    sbi SOFT_PORT_IO,SOFT_TX_BIT
    rcall .Lsoft_tx_delay
    ret
.Lsoft_tx_start_delay:
    ldi r18, SOFT_TX_START_N
.Lsoft_tx_start_loop:
    dec r18
    brne .Lsoft_tx_start_loop
    ret
.Lsoft_tx_delay:
    ldi r18, SOFT_TX_N
.Lsoft_tx_loop:
    dec r18
    brne .Lsoft_tx_loop
    ret
.endm
