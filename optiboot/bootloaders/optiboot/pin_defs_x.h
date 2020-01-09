/*
 * pin_defs.h
 * optiboot helper defining the default pin assignments (LED, SOFT_UART)
 * for the various chips that are supported.  This also has some ugly macros
 * for selecting among various UARTs and LED possibilities using command-line
 * defines like "UART=2 LED=B5"
 *
 * Copyright 2013-2020 by Bill Westfield.
 * Copyright 2010 by Peter Knight.
 * This software is licensed under version 2 of the Gnu Public Licence.
 * See optiboot.c for details.
 */


/*
 * ------------------------------------------------------------------------
 * A bunch of macros to enable the LED to be specifed as "B5" for bit 5 
 * of port B, and similar.
 * We define symbols for all the legal combination of port/bit on a chip,
 * and do pre-processor tests to see if there's a match.  This ends up
 * being very verbose, but it is pretty easy to generate semi-automatically.
 * (We wouldn't need this if the preprocessor could do string compares.)
 */

// Symbols for each PortA bit.
#define A0 0x100
#define A1 0x101
#define A2 0x102
#define A3 0x103
#define A4 0x104
#define A5 0x105
#define A6 0x106
#define A7 0x107
// If there is no PORTA on this chip, don't allow these to be used
//   (and indicate the error by redefining LED)
#if !defined(PORTA)
#if LED >= A0 && LED <= A7
#undef LED
#define LED -1
#endif
#endif

#define B0 0x200
#define B1 0x201
#define B2 0x202
#define B3 0x203
#define B4 0x204
#define B5 0x205
#define B6 0x206
#define B7 0x207
#if !defined(PORTB)
#if LED >= B0 && LED <= B7
#undef LED
#define LED -1
#endif
#endif

#define C0 0x300
#define C1 0x301
#define C2 0x302
#define C3 0x303
#define C4 0x304
#define C5 0x305
#define C6 0x306
#define C7 0x307
#if !(defined(PORTC))
#if LED >= C0 && LED <= C7
#undef LED
#define LED -1
#endif
#endif

#define D0 0x400
#define D1 0x401
#define D2 0x402
#define D3 0x403
#define D4 0x404
#define D5 0x405
#define D6 0x406
#define D7 0x407
#if !(defined(PORTD))
#if LED >= D0 && LED <= D7
#undef LED
#define LED -1
#endif
#endif

#define E0 0x500
#define E1 0x501
#define E2 0x502
#define E3 0x503
#define E4 0x504
#define E5 0x505
#define E6 0x506
#define E7 0x507
#if !(defined(PORTE))
#if LED >= E0 && LED <= E7
#undef LED
#define LED -1
#endif
#endif

#define F0 0x600
#define F1 0x601
#define F2 0x602
#define F3 0x603
#define F4 0x604
#define F5 0x605
#define F6 0x606
#define F7 0x607
#if !(defined(PORTF))
#if LED >= F0 && LED <= F7
#undef LED
#define LED -1
#endif
#endif

#define G0 0x700
#define G1 0x701
#define G2 0x702
#define G3 0x703
#define G4 0x704
#define G5 0x705
#define G6 0x706
#define G7 0x707
#if !defined(PORTG)
#if LED >= G0 && LED <= G7
#undef LED
#define LED -1
#endif
#endif

#define H0 0x800
#define H1 0x801
#define H2 0x802
#define H3 0x803
#define H4 0x804
#define H5 0x805
#define H6 0x806
#define H7 0x807
#if !(defined(PORTH))
#if LED >= H0 && LED <= H7
#undef LED
#define LED -1
#endif
#endif

#define J0 0xA00
#define J1 0xA01
#define J2 0xA02
#define J3 0xA03
#define J4 0xA04
#define J5 0xA05
#define J6 0xA06
#define J7 0xA07
#if !(defined(PORTJ))
#if LED >= J0 && LED <= J7
#undef LED
#define LED -1
#endif
#endif

#define K0 0xB00
#define K1 0xB01
#define K2 0xB02
#define K3 0xB03
#define K4 0xB04
#define K5 0xB05
#define K6 0xB06
#define K7 0xB07
#if !(defined(PORTK))
#if LED >= K0 && LED <= K7
#undef LED
#define LED -1
#endif
#endif

#define L0 0xC00
#define L1 0xC01
#define L2 0xC02
#define L3 0xC03
#define L4 0xC04
#define L5 0xC05
#define L6 0xC06
#define L7 0xC07
#if !(defined(PORTL))
#if LED >= L0 && LED <= L7
#undef LED
#define LED -1
#endif
#endif


/*
 * A statement like "#if LED == B0" will evaluation (in the preprocessor)
 * to #if C0 == B0, and then to #if 0x301 == 0x201
 */
#if LED == B0
#define LED_NAME "B0"
#undef LED
#define LED_PORT    VPORTB
#define LED	    (1<<PORT0)
#elif LED == B1
#define LED_NAME "B1"
#undef LED
#define LED_PORT    VPORTB
#define LED	    (1<<PORT1)
#elif LED == B2
#define LED_NAME "B2"
#undef LED
#define LED_PORT    VPORTB
#define LED	    (1<<PORT2)
#elif LED == B3
#define LED_NAME "B3"
#undef LED
#define LED_PORT    VPORTB
#define LED	    (1<<PORT3)
#elif LED == B4
#define LED_NAME "B4"
#undef LED
#define LED_PORT    VPORTB
#define LED	    (1<<PORT4)
#elif LED == B5
#define LED_NAME "B5"
#undef LED
#define LED_PORT    VPORTB
#define LED	    (1<<PORT5)
#elif LED == B6
#define LED_NAME "B6"
#undef LED
#define LED_PORT    VPORTB
#define LED	    (1<<PORT6)
#elif LED == B7
#define LED_NAME "B7"
#undef LED
#define LED_PORT    VPORTB
#define LED	    (1<<PORT7)

#elif LED == C0
#define LED_NAME "C0"
#undef LED
#define LED_PORT    VPORTC
#define LED	    (1<<PORT0)
#elif LED == C1
#define LED_NAME "C1"
#undef LED
#define LED_PORT    VPORTC
#define LED	    (1<<PORT1)
#elif LED == C2
#define LED_NAME "C2"
#undef LED
#define LED_PORT    VPORTC
#define LED	    (1<<PORT2)
#elif LED == C3
#define LED_NAME "C3"
#undef LED
#define LED_PORT    VPORTC
#define LED	    (1<<PORT3)
#elif LED == C4
#define LED_NAME "C4"
#undef LED
#define LED_PORT    VPORTC
#define LED	    (1<<PORT4)
#elif LED == C5
#define LED_NAME "C5"
#undef LED
#define LED_PORT    VPORTC
#define LED	    (1<<PORT5)
#elif LED == C6
#define LED_NAME "C6"
#undef LED
#define LED_PORT    VPORTC
#define LED	    (1<<PORT6)
#elif LED == C7
#define LED_NAME "C7"
#undef LED
#define LED_PORT    VPORTC
#define LED	    (1<<PORT7)

#elif LED == D0
#define LED_NAME "D0"
#undef LED
#define LED_PORT    VPORTD
#define LED	    (1<<PORT0)
#elif LED == D1
#define LED_NAME "D1"
#undef LED
#define LED_PORT    VPORTD
#define LED	    (1<<PORT1)
#elif LED == D2
#define LED_NAME "D2"
#undef LED
#define LED_PORT    VPORTD
#define LED	    (1<<PORT2)
#elif LED == D3
#define LED_NAME "D3"
#undef LED
#define LED_PORT    VPORTD
#define LED	    (1<<PORT3)
#elif LED == D4
#define LED_NAME "D4"
#undef LED
#define LED_PORT    VPORTD
#define LED	    (1<<PORT4)
#elif LED == D5
#define LED_NAME "D5"
#undef LED
#define LED_PORT    VPORTD
#define LED	    (1<<PORT5)
#elif LED == D6
#define LED_NAME "D6"
#undef LED
#define LED_PORT    VPORTD
#define LED	    (1<<PORT6)
#elif LED == D7
#define LED_NAME "D7"
#undef LED
#define LED_PORT    VPORTD
#define LED	    (1<<PORT7)

#elif LED == E0
#define LED_NAME "E0"
#undef LED
#define LED_PORT    VPORTE
#define LED	    (1<<PORT0)
#elif LED == E1
#define LED_NAME "E1"
#undef LED
#define LED_PORT    VPORTE
#define LED	    (1<<PORT1)
#elif LED == E2
#define LED_NAME "E2"
#undef LED
#define LED_PORT    VPORTE
#define LED	    (1<<PORT2)
#elif LED == E3
#define LED_NAME "E3"
#undef LED
#define LED_PORT    VPORTE
#define LED	    (1<<PORT3)
#elif LED == E4
#define LED_NAME "E4"
#undef LED
#define LED_PORT    VPORTE
#define LED	    (1<<PORT4)
#elif LED == E5
#define LED_NAME "E5"
#undef LED
#define LED_PORT    VPORTE
#define LED	    (1<<PORT5)
#elif LED == E6
#define LED_NAME "E6"
#undef LED
#define LED_PORT    VPORTE
#define LED	    (1<<PORT6)
#elif LED == E7
#define LED_NAME "E7"
#undef LED
#define LED_PORT    VPORTE
#define LED	    (1<<PORT7)

#elif LED == F0
#define LED_NAME "F0"
#undef LED
#define LED_PORT    VPORTF
#define LED	    (1<<PORT0)
#elif LED == F1
#define LED_NAME "F1"
#undef LED
#define LED_PORT    VPORTF
#define LED	    (1<<PORT1)
#elif LED == F2
#define LED_NAME "F2"
#undef LED
#define LED_PORT    VPORTF
#define LED	    (1<<PORT2)
#elif LED == F3
#define LED_NAME "F3"
#undef LED
#define LED_PORT    VPORTF
#define LED	    (1<<PORT3)
#elif LED == F4
#define LED_NAME "F4"
#undef LED
#define LED_PORT    VPORTF
#define LED	    (1<<PORT4)
#elif LED == F5
#define LED_NAME "F5"
#undef LED
#define LED_PORT    VPORTF
#define LED	    (1<<PORT5)
#elif LED == F6
#define LED_NAME "F6"
#undef LED
#define LED_PORT    VPORTF
#define LED	    (1<<PORT6)
#elif LED == F7
#define LED_NAME "F7"
#undef LED
#define LED_PORT    VPORTF
#define LED	    (1<<PORT7)

#elif LED == G0
#define LED_NAME "G0"
#undef LED
#define LED_PORT    VPORTG
#define LED	    (1<<PORT0)
#elif LED == G1
#define LED_NAME "G1"
#undef LED
#define LED_PORT    VPORTG
#define LED	    (1<<PORT1)
#elif LED == G2
#define LED_NAME "G2"
#undef LED
#define LED_PORT    VPORTG
#define LED	    (1<<PORT2)
#elif LED == G3
#define LED_NAME "G3"
#undef LED
#define LED_PORT    VPORTG
#define LED	    (1<<PORT3)
#elif LED == G4
#define LED_NAME "G4"
#undef LED
#define LED_PORT    VPORTG
#define LED	    (1<<PORT4)
#elif LED == G5
#define LED_NAME "G5"
#undef LED
#define LED_PORT    VPORTG
#define LED	    (1<<PORT5)
#elif LED == G6
#define LED_NAME "G6"
#undef LED
#define LED_PORT    VPORTG
#define LED	    (1<<PORT6)
#elif LED == G7
#define LED_NAME "G7"
#undef LED
#define LED_PORT    VPORTG
#define LED	    (1<<PORT7)

#elif LED == H0
#define LED_NAME "H0"
#undef LED
#define LED_PORT    VPORTH
#define LED	    (1<<PORT0)
#elif LED == H1
#define LED_NAME "H1"
#undef LED
#define LED_PORT    VPORTH
#define LED	    (1<<PORT1)
#elif LED == H2
#define LED_NAME "H2"
#undef LED
#define LED_PORT    VPORTH
#define LED	    (1<<PORT2)
#elif LED == H3
#define LED_NAME "H3"
#undef LED
#define LED_PORT    VPORTH
#define LED	    (1<<PORT3)
#elif LED == H4
#define LED_NAME "H4"
#undef LED
#define LED_PORT    VPORTH
#define LED	    (1<<PORT4)
#elif LED == H5
#define LED_NAME "H5"
#undef LED
#define LED_PORT    VPORTH
#define LED	    (1<<PORT5)
#elif LED == H6
#define LED_NAME "H6"
#undef LED
#define LED_PORT    VPORTH
#define LED	    (1<<PORT6)
#elif LED == H7
#define LED_NAME "H7"
#undef LED
#define LED_PORT    VPORTH
#define LED	    (1<<PORT7)

#elif LED == J0
#define LED_NAME "J0"
#undef LED
#define LED_PORT    VPORTJ
#define LED	    (1<<PORT0)
#elif LED == J1
#define LED_NAME "J1"
#undef LED
#define LED_PORT    VPORTJ
#define LED	    (1<<PORT1)
#elif LED == J2
#define LED_NAME "J2"
#undef LED
#define LED_PORT    VPORTJ
#define LED	    (1<<PORT2)
#elif LED == J3
#define LED_NAME "J3"
#undef LED
#define LED_PORT    VPORTJ
#define LED	    (1<<PORT3)
#elif LED == J4
#define LED_NAME "J4"
#undef LED
#define LED_PORT    VPORTJ
#define LED	    (1<<PORT4)
#elif LED == J5
#define LED_NAME "J5"
#undef LED
#define LED_PORT    VPORTJ
#define LED	    (1<<PORT5)
#elif LED == J6
#define LED_NAME "J6"
#undef LED
#define LED_PORT    VPORTJ
#define LED	    (1<<PORT6)
#elif LED == J7
#define LED_NAME "J7"
#undef LED
#define LED_PORT    VPORTJ
#define LED	    (1<<PORT7)

#elif LED == K0
#define LED_NAME "K0"
#undef LED
#define LED_PORT    VPORTK
#define LED	    (1<<PORT0)
#elif LED == K1
#define LED_NAME "K1"
#undef LED
#define LED_PORT    VPORTK
#define LED	    (1<<PORT1)
#elif LED == K2
#define LED_NAME "K2"
#undef LED
#define LED_PORT    VPORTK
#define LED	    (1<<PORT2)
#elif LED == K3
#define LED_NAME "K3"
#undef LED
#define LED_PORT    VPORTK
#define LED	    (1<<PORT3)
#elif LED == K4
#define LED_NAME "K4"
#undef LED
#define LED_PORT    VPORTK
#define LED	    (1<<PORT4)
#elif LED == K5
#define LED_NAME "K5"
#undef LED
#define LED_PORT    VPORTK
#define LED	    (1<<PORT5)
#elif LED == K6
#define LED_NAME "K6"
#undef LED
#define LED_PORT    VPORTK
#define LED	    (1<<PORT6)
#elif LED == K7
#define LED_NAME "K7"
#undef LED
#define LED_PORT    VPORTK
#define LED	    (1<<PORT7)

#elif LED == L0
#define LED_NAME "L0"
#undef LED
#define LED_PORT    VPORTL
#define LED	    (1<<PORT0)
#elif LED == L1
#define LED_NAME "L1"
#undef LED
#define LED_PORT    VPORTL
#define LED	    (1<<PORT1)
#elif LED == L2
#define LED_NAME "L2"
#undef LED
#define LED_PORT    VPORTL
#define LED	    (1<<PORT2)
#elif LED == L3
#define LED_NAME "L3"
#undef LED
#define LED_PORT    VPORTL
#define LED	    (1<<PORT3)
#elif LED == L4
#define LED_NAME "L4"
#undef LED
#define LED_PORT    VPORTL
#define LED	    (1<<PORT4)
#elif LED == L5
#define LED_NAME "L5"
#undef LED
#define LED_PORT    VPORTL
#define LED	    (1<<PORT5)
#elif LED == L6
#define LED_NAME "L6"
#undef LED
#define LED_PORT    VPORTL
#define LED	    (1<<PORT6)
#elif LED == L7
#define LED_NAME "L7"
#undef LED
#define LED_PORT    VPORTL
#define LED	    (1<<PORT7)

#elif LED == A0
#define LED_NAME "A0"
#undef LED
#ifdef VPORTA
#define LED_PORT    VPORTA.OUT
#define LED_PORT     VPORTA.IN
#else
#define LED_PORT    VPORTA
#endif
#define LED	    (1<<PORT0)
#elif LED == A1
#define LED_NAME "A1"
#undef LED
#define LED_PORT    VPORTA
#define LED	    (1<<PORT1)
#elif LED == A2
#define LED_NAME "A2"
#undef LED
#define LED_PORT    VPORTA
#define LED	    (1<<PORT2)
#elif LED == A3
#define LED_NAME "A3"
#undef LED
#define LED_PORT    VPORTA
#define LED	    (1<<PORT3)
#elif LED == A4
#define LED_NAME "A4"
#undef LED
#define LED_PORT    VPORTA
#define LED	    (1<<PORT4)
#elif LED == A5
#define LED_NAME "A5"
#undef LED
#define LED_PORT    VPORTA
#define LED	    (1<<PORT5)
#elif LED == A6
#define LED_NAME "A6"
#undef LED
#define LED_PORT    VPORTA
#define LED	    (1<<PORT6)
#elif LED == A7
#define LED_NAME "A7"
#undef LED
#define LED_PORT    VPORTA
#define LED	    (1<<PORT7)

#else
// Stop compilation right away, so we don't get more errors.
#if LED == -1
#error Unrecognized LED name.  Should be like "B5"
// Stop compilation right away, so we don't get more errors.
#pragma GCC diagnostic warning "-Wfatal-errors"
#error Nonexistent LED PORT.  Check datasheet.
#endif
#pragma GCC diagnostic warning "-Wfatal-errors"
#error Unrecognized LED name.  Should be like "B5"
#endif

/*
 * Handle devices with up to 4 uarts.  Rather inelegantly.
 */
#define USART_ALTPMUX 1

#if defined(__AVR_ATmega4809__) || defined(__AVR_ATmega4808__) || \
    defined(__AVR_ATmega3209__) || defined(__AVR_ATmega3208__) || \
    defined(__AVR_ATmega1609__) || defined(__AVR_ATmega1608__) || \
    defined(__AVR_ATmega809__) || defined(__AVR_ATmega808__)
#define MYPMUX_REG PORTMUX.USARTROUTEA
# if (UARTTX == A0)
#  define UART_NAME "A0"
#  ifndef USART0
#   error Pin on USART0, but no USART0 exists
#  endif
#  define MYUART USART0
#  define MYUART_TXPORT VPORTA
#  define MYUART_TXPIN (1<<PORT0)
#  define MYUART_PMUX_VAL (PORTMUX_USART0_DEFAULT_gc)
# elif (UARTTX == A4)
#  define UART_NAME "A4"
#  ifndef USART0
#   error Pin on USART0, but no USART0 exists
#  endif
#  define MYUART USART0
#  define MYUART_TXPORT VPORTA
#  define MYUART_TXPIN (1<<PORT4)
#  define MYUART_PMUX_VAL (PORTMUX_USART0_ALT1_gc)
# elif (UARTTX == B0)
#  define UART_NAME "B0"
#  ifndef USART3
#   error Pin on USART3, but no USART3 exists
#  endif
#  define MYUART USART3
#  define MYUART_TXPORT VPORTB
#  define MYUART_TXPIN (1<<PORT0)
#  define MYUART_PMUX_VAL (PORTMUX_USART3_DEFAULT_gc)
# elif (UARTTX == B4)
#  define UART_NAME "B4"
#  ifndef USART3
#   error Pin on USART3, but no USART3 exists
#  endif
#  define MYUART USART3
#  define MYUART_TXPORT VPORTB
#  define MYUART_TXPIN (1<<PORT4)
#  define MYUART_PMUX_VAL (PORTMUX_USART3_ALT1_gc)
# elif (UARTTX == C0)
#  define UART_NAME "C0"
#  ifndef USART1
#   error Pin on USART1, but no USART1 exists
#  endif
#  define MYUART USART1
#  define MYUART_TXPORT VPORTC
#  define MYUART_TXPIN (1<<PORT0)
#  define MYUART_PMUX_VAL (PORTMUX_USART1_DEFAULT_gc)
# elif (UARTTX == C4)
#  define UART_NAME "C4"
#  ifndef USART1
#   error Pin on USART1, but no USART1 exists
#  endif
#  define MYUART USART1
#  define MYUART_TXPORT VPORTC
#  define MYUART_TXPIN (1<<PORT4)
#  define MYUART_PMUX_VAL (PORTMUX_USART1_ALT1_gc)
# elif (UARTTX == F0)
#  define UART_NAME "F0"
#  ifndef USART2
#   error Pin on USART2, but no USART2 exists
#  endif
#  define MYUART USART2
#  define MYUART_TXPORT VPORTF
#  define MYUART_TXPIN (1<<PORT0)
#  define MYUART_PMUX_VAL (PORTMUX_USART2_DEFAULT_gc)
# elif (UARTTX == F4)
#  define UART_NAME "F4"
#  ifndef USART2
#   error Pin on USART2, but no USART2 exists
#  endif
#  define MYUART USART2
#  define MYUART_TXPORT VPORTF
#  define MYUART_TXPIN (1<<PORT4)
#  define MYUART_PMUX_VAL (PORTMUX_USART2_ALT1_gc)
# else
#  pragma GCC diagnostic warning "-Wfatal-errors"
#  pragma GCC error "Invalid UARTTX pin for chip"
# endif
#endif  // ATmega4809

/*
 * 8pin Tiny0 and Tiny1
 */
#if defined(__AVR_ATtiny402__) || defined(__AVR_ATtiny202__) || \
    defined(__AVR_ATtiny412__) || defined(__AVR_ATtiny212__)
#define MYPMUX_REG PORTMUX.CTRLB
# if (UARTTX == A6)
#  define UART_NAME "A6"
#  ifndef USART0
#   error Pin on USART0, but no USART0 exists
#  endif
#  define MYUART USART0
#  define MYUART_TXPORT VPORTA
#  define MYUART_TXPIN (1<<PORT6)
#  define MYUART_PMUX_VAL 0
# elif (UARTTX == A1)
#  define UART_NAME "A1"
#  ifndef USART0
#   error Pin on USART0, but no USART0 exists
#  endif
#  define MYUART USART0
#  define MYUART_TXPORT VPORTA
#  define MYUART_TXPIN (1<<PORT1)
#  define MYUART_PMUX_VAL (USART_ALTPMUX)
# else
#  pragma GCC diagnostic warning "-Wfatal-errors"
#  pragma GCC error "Invalid UARTTX pin for chip"
# endif
#endif // Tiny402/etc

/*
 *  14, 20, and 24 pin  Tiny0, Tiny1
 *   The 14, 20, and 24pin packages all conveniently have the UART on the
 *   same port pins, and the same pinmux structure!
 */
#if defined(__AVR_ATtiny1614__) || defined(__AVR_ATtiny1604__) ||	\
    defined(__AVR_ATtiny814__)  || defined(__AVR_ATtiny804__) ||	\
    defined(__AVR_ATtiny1606__) || defined(__AVR_ATtiny806__) ||	\
    defined(__AVR_ATtiny406__) || defined(__AVR_ATtiny3216__) ||	\
    defined(__AVR_ATtiny816__) || defined(__AVR_ATtiny416__) || \
    defined(__AVR_ATtiny1617__) || defined(__AVR_ATtiny3217__) || \
    defined(__AVR_ATtiny1607__) || defined(__AVR_ATtiny817__)
#define MYPMUX_REG PORTMUX.CTRLB
# if (UARTTX == B2)
#  define UART_NAME "B2"
#  ifndef USART0
#   error Pin on USART0, but no USART0 exists
#  endif
#  define MYUART USART0
#  define MYUART_TXPORT VPORTB
#  define MYUART_TXPIN (1<<PORT2)
#  define MYUART_PMUX_VAL 0
# elif (UARTTX == A1)
#  define UART_NAME "A1"
#  ifndef USART0
#   error Pin on USART0, but no USART0 exists
#  endif
#  define MYUART USART0
#  define MYUART_TXPORT VPORTA
#  define MYUART_TXPIN (1<<PORT1)
#  define MYUART_PMUX_VAL (USART_ALTPMUX)
# else
#  pragma GCC diagnostic warning "-Wfatal-errors"
#  pragma GCC error "Invalid UARTTX pin for chip"
# endif
#endif

#ifndef MYUART
# warning No UARTTX pin specified.
#endif
