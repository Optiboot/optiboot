#undef UART_PORT
#undef UART_PIN
#undef UART_DDR
#undef UART_TX_BIT
#undef UART_RX_BIT

#if SOFTTX == B0
#define SOFTTX_NAME "B0"
#define UART_DDR     DDRB
#define UART_PORT    PORTB
#define UART_TX_BIT  PINB0
#elif SOFTTX == B1
#define SOFTTX_NAME "B1"
#define UART_DDR     DDRB
#define UART_PORT      PORTB
#define UART_TX_BIT    PINB1
#elif SOFTTX == B2
#define SOFTTX_NAME "B2"
#define UART_DDR     DDRB
#define UART_PORT      PORTB
#define UART_TX_BIT    PINB2
#elif SOFTTX == B3
#define SOFTTX_NAME "B3"
#define UART_DDR     DDRB
#define UART_PORT      PORTB
#define UART_TX_BIT    PINB3
#elif SOFTTX == B4
#define SOFTTX_NAME "B4"
#define UART_DDR     DDRB
#define UART_PORT      PORTB
#define UART_TX_BIT    PINB4
#elif SOFTTX == B5
#define SOFTTX_NAME "B5"
#define UART_DDR     DDRB
#define UART_PORT      PORTB
#define UART_TX_BIT    PINB5
#elif SOFTTX == B6
#define SOFTTX_NAME "B6"
#define UART_DDR     DDRB
#define UART_PORT      PORTB
#define UART_TX_BIT    PINB6
#elif SOFTTX == B7
#define SOFTTX_NAME "B7"
#define UART_DDR     DDRB
#define UART_PORT      PORTB
#define UART_TX_BIT    PINB7

#elif SOFTTX == C0
#define SOFTTX_NAME "C0"
#define UART_DDR     DDRC
#define UART_PORT      PORTC
#define UART_TX_BIT    PINC0
#elif SOFTTX == C1
#define SOFTTX_NAME "C1"
#define UART_DDR     DDRC
#define UART_PORT      PORTC
#define UART_TX_BIT    PINC1
#elif SOFTTX == C2
#define SOFTTX_NAME "C2"
#define UART_DDR     DDRC
#define UART_PORT      PORTC
#define UART_TX_BIT    PINC2
#elif SOFTTX == C3
#define SOFTTX_NAME "C3"
#define UART_DDR     DDRC
#define UART_PORT      PORTC
#define UART_TX_BIT    PINC3
#elif SOFTTX == C4
#define SOFTTX_NAME "C4"
#define UART_DDR     DDRC
#define UART_PORT      PORTC
#define UART_TX_BIT    PINC4
#elif SOFTTX == C5
#define SOFTTX_NAME "C5"
#define UART_DDR     DDRC
#define UART_PORT      PORTC
#define UART_TX_BIT    PINC5
#elif SOFTTX == C6
#define SOFTTX_NAME "C6"
#define UART_DDR     DDRC
#define UART_PORT      PORTC
#define UART_TX_BIT    PINC6
#elif SOFTTX == C7
#define SOFTTX_NAME "C7"
#define UART_DDR     DDRC
#define UART_PORT      PORTC
#define UART_TX_BIT    PINC7

#elif SOFTTX == D0
#define SOFTTX_NAME "D0"
#define UART_DDR     DDRD
#define UART_PORT      PORTD
#define UART_TX_BIT    PIND0
#elif SOFTTX == D1
#define SOFTTX_NAME "D1"
#define UART_DDR     DDRD
#define UART_PORT      PORTD
#define UART_TX_BIT    PIND1
#elif SOFTTX == D2
#define SOFTTX_NAME "D2"
#define UART_DDR     DDRD
#define UART_PORT      PORTD
#define UART_TX_BIT    PIND2
#elif SOFTTX == D3
#define SOFTTX_NAME "D3"
#define UART_DDR     DDRD
#define UART_PORT      PORTD
#define UART_TX_BIT    PIND3
#elif SOFTTX == D4
#define SOFTTX_NAME "D4"
#define UART_DDR     DDRD
#define UART_PORT      PORTD
#define UART_TX_BIT    PIND4
#elif SOFTTX == D5
#define SOFTTX_NAME "D5"
#define UART_DDR     DDRD
#define UART_PORT      PORTD
#define UART_TX_BIT    PIND5
#elif SOFTTX == D6
#define SOFTTX_NAME "D6"
#define UART_DDR     DDRD
#define UART_PORT      PORTD
#define UART_TX_BIT    PIND6
#elif SOFTTX == D7
#define SOFTTX_NAME "D7"
#define UART_DDR     DDRD
#define UART_PORT      PORTD
#define UART_TX_BIT    PIND7

#elif SOFTTX == E0
#define SOFTTX_NAME "E0"
#define UART_DDR     DDRE
#define UART_PORT      PORTE
#define UART_TX_BIT    PINE0
#elif SOFTTX == E1
#define SOFTTX_NAME "E1"
#define UART_DDR     DDRE
#define UART_PORT      PORTE
#define UART_TX_BIT    PINE1
#elif SOFTTX == E2
#define SOFTTX_NAME "E2"
#define UART_DDR     DDRE
#define UART_PORT      PORTE
#define UART_TX_BIT    PINE2
#elif SOFTTX == E3
#define SOFTTX_NAME "E3"
#define UART_DDR     DDRE
#define UART_PORT      PORTE
#define UART_TX_BIT    PINE3
#elif SOFTTX == E4
#define SOFTTX_NAME "E4"
#define UART_DDR     DDRE
#define UART_PORT      PORTE
#define UART_TX_BIT    PINE4
#elif SOFTTX == E5
#define SOFTTX_NAME "E5"
#define UART_DDR     DDRE
#define UART_PORT      PORTE
#define UART_TX_BIT    PINE5
#elif SOFTTX == E6
#define SOFTTX_NAME "E6"
#define UART_DDR     DDRE
#define UART_PORT      PORTE
#define UART_TX_BIT    PINE6
#elif SOFTTX == E7
#define SOFTTX_NAME "E7"
#define UART_DDR     DDRE
#define UART_PORT      PORTE
#define UART_TX_BIT    PINE7

#elif SOFTTX == F0
#define SOFTTX_NAME "F0"
#define UART_DDR     DDRF
#define UART_PORT      PORTF
#define UART_TX_BIT    PINF0
#elif SOFTTX == F1
#define SOFTTX_NAME "F1"
#define UART_DDR     DDRF
#define UART_PORT      PORTF
#define UART_TX_BIT    PINF1
#elif SOFTTX == F2
#define SOFTTX_NAME "F2"
#define UART_DDR     DDRF
#define UART_PORT      PORTF
#define UART_TX_BIT    PINF2
#elif SOFTTX == F3
#define SOFTTX_NAME "F3"
#define UART_DDR     DDRF
#define UART_PORT      PORTF
#define UART_TX_BIT    PINF3
#elif SOFTTX == F4
#define SOFTTX_NAME "F4"
#define UART_DDR     DDRF
#define UART_PORT      PORTF
#define UART_TX_BIT    PINF4
#elif SOFTTX == F5
#define SOFTTX_NAME "F5"
#define UART_DDR     DDRF
#define UART_PORT      PORTF
#define UART_TX_BIT    PINF5
#elif SOFTTX == F6
#define SOFTTX_NAME "F6"
#define UART_DDR     DDRF
#define UART_PORT      PORTF
#define UART_TX_BIT    PINF6
#elif SOFTTX == F7
#define SOFTTX_NAME "F7"
#define UART_DDR     DDRF
#define UART_PORT      PORTF
#define UART_TX_BIT    PINF7

#elif SOFTTX == G0
#define SOFTTX_NAME "G0"
#define UART_DDR     DDRG
#define UART_PORT      PORTG
#define UART_TX_BIT    PING0
#elif SOFTTX == G1
#define SOFTTX_NAME "G1"
#define UART_DDR     DDRG
#define UART_PORT      PORTG
#define UART_TX_BIT    PING1
#elif SOFTTX == G2
#define SOFTTX_NAME "G2"
#define UART_DDR     DDRG
#define UART_PORT      PORTG
#define UART_TX_BIT    PING2
#elif SOFTTX == G3
#define SOFTTX_NAME "G3"
#define UART_DDR     DDRG
#define UART_PORT      PORTG
#define UART_TX_BIT    PING3
#elif SOFTTX == G4
#define SOFTTX_NAME "G4"
#define UART_DDR     DDRG
#define UART_PORT      PORTG
#define UART_TX_BIT    PING4
#elif SOFTTX == G5
#define SOFTTX_NAME "G5"
#define UART_DDR     DDRG
#define UART_PORT      PORTG
#define UART_TX_BIT    PING5
#elif SOFTTX == G6
#define SOFTTX_NAME "G6"
#define UART_DDR     DDRG
#define UART_PORT      PORTG
#define UART_TX_BIT    PING6
#elif SOFTTX == G7
#define SOFTTX_NAME "G7"
#define UART_DDR     DDRG
#define UART_PORT      PORTG
#define UART_TX_BIT    PING7

#elif SOFTTX == H0
#define SOFTTX_NAME "H0"
#define UART_DDR     DDRH
#define UART_PORT      PORTH
#define UART_TX_BIT    PINH0
#elif SOFTTX == H1
#define SOFTTX_NAME "H1"
#define UART_DDR     DDRH
#define UART_PORT      PORTH
#define UART_TX_BIT    PINH1
#elif SOFTTX == H2
#define SOFTTX_NAME "H2"
#define UART_DDR     DDRH
#define UART_PORT      PORTH
#define UART_TX_BIT    PINH2
#elif SOFTTX == H3
#define SOFTTX_NAME "H3"
#define UART_DDR     DDRH
#define UART_PORT      PORTH
#define UART_TX_BIT    PINH3
#elif SOFTTX == H4
#define SOFTTX_NAME "H4"
#define UART_DDR     DDRH
#define UART_PORT      PORTH
#define UART_TX_BIT    PINH4
#elif SOFTTX == H5
#define SOFTTX_NAME "H5"
#define UART_DDR     DDRH
#define UART_PORT      PORTH
#define UART_TX_BIT    PINH5
#elif SOFTTX == H6
#define SOFTTX_NAME "H6"
#define UART_DDR     DDRH
#define UART_PORT      PORTH
#define UART_TX_BIT    PINH6
#elif SOFTTX == H7
#define SOFTTX_NAME "H7"
#define UART_DDR     DDRH
#define UART_PORT      PORTH
#define UART_TX_BIT    PINH7

#elif SOFTTX == J0
#define SOFTTX_NAME "J0"
#define UART_DDR     DDRJ
#define UART_PORT      PORTJ
#define UART_TX_BIT    PINJ0
#elif SOFTTX == J1
#define SOFTTX_NAME "J1"
#define UART_DDR     DDRJ
#define UART_PORT      PORTJ
#define UART_TX_BIT    PINJ1
#elif SOFTTX == J2
#define SOFTTX_NAME "J2"
#define UART_DDR     DDRJ
#define UART_PORT      PORTJ
#define UART_TX_BIT    PINJ2
#elif SOFTTX == J3
#define SOFTTX_NAME "J3"
#define UART_DDR     DDRJ
#define UART_PORT      PORTJ
#define UART_TX_BIT    PINJ3
#elif SOFTTX == J4
#define SOFTTX_NAME "J4"
#define UART_DDR     DDRJ
#define UART_PORT      PORTJ
#define UART_TX_BIT    PINJ4
#elif SOFTTX == J5
#define SOFTTX_NAME "J5"
#define UART_DDR     DDRJ
#define UART_PORT      PORTJ
#define UART_TX_BIT    PINJ5
#elif SOFTTX == J6
#define SOFTTX_NAME "J6"
#define UART_DDR     DDRJ
#define UART_PORT      PORTJ
#define UART_TX_BIT    PINJ6
#elif SOFTTX == J7
#define SOFTTX_NAME "J7"
#define UART_DDR     DDRJ
#define UART_PORT      PORTJ
#define UART_TX_BIT    PINJ7

#elif SOFTTX == K0
#define SOFTTX_NAME "K0"
#define UART_DDR     DDRK
#define UART_PORT      PORTK
#define UART_TX_BIT    PINK0
#elif SOFTTX == K1
#define SOFTTX_NAME "K1"
#define UART_DDR     DDRK
#define UART_PORT      PORTK
#define UART_TX_BIT    PINK1
#elif SOFTTX == K2
#define SOFTTX_NAME "K2"
#define UART_DDR     DDRK
#define UART_PORT      PORTK
#define UART_TX_BIT    PINK2
#elif SOFTTX == K3
#define SOFTTX_NAME "K3"
#define UART_DDR     DDRK
#define UART_PORT      PORTK
#define UART_TX_BIT    PINK3
#elif SOFTTX == K4
#define SOFTTX_NAME "K4"
#define UART_DDR     DDRK
#define UART_PORT      PORTK
#define UART_TX_BIT    PINK4
#elif SOFTTX == K5
#define SOFTTX_NAME "K5"
#define UART_DDR     DDRK
#define UART_PORT      PORTK
#define UART_TX_BIT    PINK5
#elif SOFTTX == K6
#define SOFTTX_NAME "K6"
#define UART_DDR     DDRK
#define UART_PORT      PORTK
#define UART_TX_BIT    PINK6
#elif SOFTTX == K7
#define SOFTTX_NAME "K7"
#define UART_DDR     DDRK
#define UART_PORT      PORTK
#define UART_TX_BIT    PINK7

#elif SOFTTX == L0
#define SOFTTX_NAME "L0"
#define UART_DDR     DDRL
#define UART_PORT      PORTL
#define UART_TX_BIT    PINL0
#elif SOFTTX == L1
#define SOFTTX_NAME "L1"
#define UART_DDR     DDRL
#define UART_PORT      PORTL
#define UART_TX_BIT    PINL1
#elif SOFTTX == L2
#define SOFTTX_NAME "L2"
#define UART_DDR     DDRL
#define UART_PORT      PORTL
#define UART_TX_BIT    PINL2
#elif SOFTTX == L3
#define SOFTTX_NAME "L3"
#define UART_DDR     DDRL
#define UART_PORT      PORTL
#define UART_TX_BIT    PINL3
#elif SOFTTX == L4
#define SOFTTX_NAME "L4"
#define UART_DDR     DDRL
#define UART_PORT      PORTL
#define UART_TX_BIT    PINL4
#elif SOFTTX == L5
#define SOFTTX_NAME "L5"
#define UART_DDR     DDRL
#define UART_PORT      PORTL
#define UART_TX_BIT    PINL5
#elif SOFTTX == L6
#define SOFTTX_NAME "L6"
#define UART_DDR     DDRL
#define UART_PORT      PORTL
#define UART_TX_BIT    PINL6
#elif SOFTTX == L7
#define SOFTTX_NAME "L7"
#define UART_DDR     DDRL
#define UART_PORT      PORTL
#define UART_TX_BIT    PINL7

#elif SOFTTX == A0
#define SOFTTX_NAME "A0"
#define UART_DDR     DDRA
#define UART_PORT      PORTA
#define UART_TX_BIT    PINA0
#elif SOFTTX == A1
#define SOFTTX_NAME "A1"
#define UART_DDR     DDRA
#define UART_PORT      PORTA
#define UART_TX_BIT    PINA1
#elif SOFTTX == A2
#define SOFTTX_NAME "A2"
#define UART_DDR     DDRA
#define UART_PORT      PORTA
#define UART_TX_BIT    PINA2
#elif SOFTTX == A3
#define SOFTTX_NAME "A3"
#define UART_DDR     DDRA
#define UART_PORT      PORTA
#define UART_TX_BIT    PINA3
#elif SOFTTX == A4
#define SOFTTX_NAME "A4"
#define UART_DDR     DDRA
#define UART_PORT      PORTA
#define UART_TX_BIT    PINA4
#elif SOFTTX == A5
#define SOFTTX_NAME "A5"
#define UART_DDR     DDRA
#define UART_PORT      PORTA
#define UART_TX_BIT    PINA5
#elif SOFTTX == A6
#define SOFTTX_NAME "A6"
#define UART_DDR     DDRA
#define UART_PORT    PORTA
#define UART_TX_BIT  PINA6
#elif SOFTTX == A7
#define SOFTTX_NAME "A7"
#define UART_DDR     DDRA
#define UART_PORT    PORTA
#define UART_TX_BIT  PINA7
#else
#error -------------------------------------------
#error Unrecognized SOFTTX name.  Should be like "B5"
#error -------------------------------------------
#endif


#if SOFTRX == B0
#define SOFTRX_NAME "B0"
#define UART_PIN       PINB
#define UART_RX_BIT    PINB0
#elif SOFTRX == B1
#define SOFTRX_NAME "B1"
#define UART_PIN       PINB
#define UART_RX_BIT    PINB1
#elif SOFTRX == B2
#define SOFTRX_NAME "B2"
#define UART_PIN       PINB
#define UART_RX_BIT    PINB2
#elif SOFTRX == B3
#define SOFTRX_NAME "B3"
#define UART_PIN       PINB
#define UART_RX_BIT    PINB3
#elif SOFTRX == B4
#define SOFTRX_NAME "B4"
#define UART_PIN       PINB
#define UART_RX_BIT    PINB4
#elif SOFTRX == B5
#define SOFTRX_NAME "B5"
#define UART_PIN       PINB
#define UART_RX_BIT    PINB5
#elif SOFTRX == B6
#define SOFTRX_NAME "B6"
#define UART_PIN       PINB
#define UART_RX_BIT    PINB6
#elif SOFTRX == B7
#define SOFTRX_NAME "B7"
#define UART_PIN       PINB
#define UART_RX_BIT    PINB7

#elif SOFTRX == C0
#define SOFTRX_NAME "C0"
#define UART_PIN       PINC
#define UART_RX_BIT    PINC0
#elif SOFTRX == C1
#define SOFTRX_NAME "C1"
#define UART_PIN       PINC
#define UART_RX_BIT    PINC1
#elif SOFTRX == C2
#define SOFTRX_NAME "C2"
#define UART_PIN       PINC
#define UART_RX_BIT    PINC2
#elif SOFTRX == C3
#define SOFTRX_NAME "C3"
#define UART_PIN       PINC
#define UART_RX_BIT    PINC3
#elif SOFTRX == C4
#define SOFTRX_NAME "C4"
#define UART_PIN       PINC
#define UART_RX_BIT    PINC4
#elif SOFTRX == C5
#define SOFTRX_NAME "C5"
#define UART_PIN       PINC
#define UART_RX_BIT    PINC5
#elif SOFTRX == C6
#define SOFTRX_NAME "C6"
#define UART_PIN       PINC
#define UART_RX_BIT    PINC6
#elif SOFTRX == C7
#define SOFTRX_NAME "C7"
#define UART_PIN       PINC
#define UART_RX_BIT    PINC7

#elif SOFTRX == D0
#define SOFTRX_NAME "D0"
#define UART_PIN       PIND
#define UART_RX_BIT    PIND0
#elif SOFTRX == D1
#define SOFTRX_NAME "D1"
#define UART_PIN       PIND
#define UART_RX_BIT    PIND1
#elif SOFTRX == D2
#define SOFTRX_NAME "D2"
#define UART_PIN       PIND
#define UART_RX_BIT    PIND2
#elif SOFTRX == D3
#define SOFTRX_NAME "D3"
#define UART_PIN       PIND
#define UART_RX_BIT    PIND3
#elif SOFTRX == D4
#define SOFTRX_NAME "D4"
#define UART_PIN       PIND
#define UART_RX_BIT    PIND4
#elif SOFTRX == D5
#define SOFTRX_NAME "D5"
#define UART_PIN       PIND
#define UART_RX_BIT    PIND5
#elif SOFTRX == D6
#define SOFTRX_NAME "D6"
#define UART_PIN       PIND
#define UART_RX_BIT    PIND6
#elif SOFTRX == D7
#define SOFTRX_NAME "D7"
#define UART_PIN       PIND
#define UART_RX_BIT    PIND7

#elif SOFTRX == E0
#define SOFTRX_NAME "E0"
#define UART_PIN       PINE
#define UART_RX_BIT    PINE0
#elif SOFTRX == E1
#define SOFTRX_NAME "E1"
#define UART_PIN       PINE
#define UART_RX_BIT    PINE1
#elif SOFTRX == E2
#define SOFTRX_NAME "E2"
#define UART_PIN       PINE
#define UART_RX_BIT    PINE2
#elif SOFTRX == E3
#define SOFTRX_NAME "E3"
#define UART_PIN       PINE
#define UART_RX_BIT    PINE3
#elif SOFTRX == E4
#define SOFTRX_NAME "E4"
#define UART_PIN       PINE
#define UART_RX_BIT    PINE4
#elif SOFTRX == E5
#define SOFTRX_NAME "E5"
#define UART_PIN       PINE
#define UART_RX_BIT    PINE5
#elif SOFTRX == E6
#define SOFTRX_NAME "E6"
#define UART_PIN       PINE
#define UART_RX_BIT    PINE6
#elif SOFTRX == E7
#define SOFTRX_NAME "E7"
#define UART_PIN       PINE
#define UART_RX_BIT    PINE7

#elif SOFTRX == F0
#define SOFTRX_NAME "F0"
#define UART_PIN       PINF
#define UART_RX_BIT    PINF0
#elif SOFTRX == F1
#define SOFTRX_NAME "F1"
#define UART_PIN       PINF
#define UART_RX_BIT    PINF1
#elif SOFTRX == F2
#define SOFTRX_NAME "F2"
#define UART_PIN       PINF
#define UART_RX_BIT    PINF2
#elif SOFTRX == F3
#define SOFTRX_NAME "F3"
#define UART_PIN       PINF
#define UART_RX_BIT    PINF3
#elif SOFTRX == F4
#define SOFTRX_NAME "F4"
#define UART_PIN       PINF
#define UART_RX_BIT    PINF4
#elif SOFTRX == F5
#define SOFTRX_NAME "F5"
#define UART_PIN       PINF
#define UART_RX_BIT    PINF5
#elif SOFTRX == F6
#define SOFTRX_NAME "F6"
#define UART_PIN       PINF
#define UART_RX_BIT    PINF6
#elif SOFTRX == F7
#define SOFTRX_NAME "F7"
#define UART_PIN       PINF
#define UART_RX_BIT    PINF7

#elif SOFTRX == G0
#define SOFTRX_NAME "G0"
#define UART_PIN       PING
#define UART_RX_BIT    PING0
#elif SOFTRX == G1
#define SOFTRX_NAME "G1"
#define UART_PIN       PING
#define UART_RX_BIT    PING1
#elif SOFTRX == G2
#define SOFTRX_NAME "G2"
#define UART_PIN       PING
#define UART_RX_BIT    PING2
#elif SOFTRX == G3
#define SOFTRX_NAME "G3"
#define UART_PIN       PING
#define UART_RX_BIT    PING3
#elif SOFTRX == G4
#define SOFTRX_NAME "G4"
#define UART_PIN       PING
#define UART_RX_BIT    PING4
#elif SOFTRX == G5
#define SOFTRX_NAME "G5"
#define UART_PIN       PING
#define UART_RX_BIT    PING5
#elif SOFTRX == G6
#define SOFTRX_NAME "G6"
#define UART_PIN       PING
#define UART_RX_BIT    PING6
#elif SOFTRX == G7
#define SOFTRX_NAME "G7"
#define UART_PIN       PING
#define UART_RX_BIT    PING7

#elif SOFTRX == H0
#define SOFTRX_NAME "H0"
#define UART_PIN       PINH
#define UART_RX_BIT    PINH0
#elif SOFTRX == H1
#define SOFTRX_NAME "H1"
#define UART_PIN       PINH
#define UART_RX_BIT    PINH1
#elif SOFTRX == H2
#define SOFTRX_NAME "H2"
#define UART_PIN       PINH
#define UART_RX_BIT    PINH2
#elif SOFTRX == H3
#define SOFTRX_NAME "H3"
#define UART_PIN       PINH
#define UART_RX_BIT    PINH3
#elif SOFTRX == H4
#define SOFTRX_NAME "H4"
#define UART_PIN       PINH
#define UART_RX_BIT    PINH4
#elif SOFTRX == H5
#define SOFTRX_NAME "H5"
#define UART_PIN       PINH
#define UART_RX_BIT    PINH5
#elif SOFTRX == H6
#define SOFTRX_NAME "H6"
#define UART_PIN       PINH
#define UART_RX_BIT    PINH6
#elif SOFTRX == H7
#define SOFTRX_NAME "H7"
#define UART_PIN       PINH
#define UART_RX_BIT    PINH7

#elif SOFTRX == J0
#define SOFTRX_NAME "J0"
#define UART_PIN       PINJ
#define UART_RX_BIT    PINJ0
#elif SOFTRX == J1
#define SOFTRX_NAME "J1"
#define UART_PIN       PINJ
#define UART_RX_BIT    PINJ1
#elif SOFTRX == J2
#define SOFTRX_NAME "J2"
#define UART_PIN       PINJ
#define UART_RX_BIT    PINJ2
#elif SOFTRX == J3
#define SOFTRX_NAME "J3"
#define UART_PIN       PINJ
#define UART_RX_BIT    PINJ3
#elif SOFTRX == J4
#define SOFTRX_NAME "J4"
#define UART_PIN       PINJ
#define UART_RX_BIT    PINJ4
#elif SOFTRX == J5
#define SOFTRX_NAME "J5"
#define UART_PIN       PINJ
#define UART_RX_BIT    PINJ5
#elif SOFTRX == J6
#define SOFTRX_NAME "J6"
#define UART_PIN       PINJ
#define UART_RX_BIT    PINJ6
#elif SOFTRX == J7
#define SOFTRX_NAME "J7"
#define UART_PIN       PINJ
#define UART_RX_BIT    PINJ7

#elif SOFTRX == K0
#define SOFTRX_NAME "K0"
#define UART_PIN       PINK
#define UART_RX_BIT    PINK0
#elif SOFTRX == K1
#define SOFTRX_NAME "K1"
#define UART_PIN       PINK
#define UART_RX_BIT    PINK1
#elif SOFTRX == K2
#define SOFTRX_NAME "K2"
#define UART_PIN       PINK
#define UART_RX_BIT    PINK2
#elif SOFTRX == K3
#define SOFTRX_NAME "K3"
#define UART_PIN       PINK
#define UART_RX_BIT    PINK3
#elif SOFTRX == K4
#define SOFTRX_NAME "K4"
#define UART_PIN       PINK
#define UART_RX_BIT    PINK4
#elif SOFTRX == K5
#define SOFTRX_NAME "K5"
#define UART_PIN       PINK
#define UART_RX_BIT    PINK5
#elif SOFTRX == K6
#define SOFTRX_NAME "K6"
#define UART_PIN       PINK
#define UART_RX_BIT    PINK6
#elif SOFTRX == K7
#define SOFTRX_NAME "K7"
#define UART_PIN       PINK
#define UART_RX_BIT    PINK7

#elif SOFTRX == L0
#define SOFTRX_NAME "L0"
#define UART_PIN       PINL
#define UART_RX_BIT    PINL0
#elif SOFTRX == L1
#define SOFTRX_NAME "L1"
#define UART_PIN       PINL
#define UART_RX_BIT    PINL1
#elif SOFTRX == L2
#define SOFTRX_NAME "L2"
#define UART_PIN       PINL
#define UART_RX_BIT    PINL2
#elif SOFTRX == L3
#define SOFTRX_NAME "L3"
#define UART_PIN       PINL
#define UART_RX_BIT    PINL3
#elif SOFTRX == L4
#define SOFTRX_NAME "L4"
#define UART_PIN       PINL
#define UART_RX_BIT    PINL4
#elif SOFTRX == L5
#define SOFTRX_NAME "L5"
#define UART_PIN       PINL
#define UART_RX_BIT    PINL5
#elif SOFTRX == L6
#define SOFTRX_NAME "L6"
#define UART_PIN       PINL
#define UART_RX_BIT    PINL6
#elif SOFTRX == L7
#define SOFTRX_NAME "L7"
#define UART_PIN       PINL
#define UART_RX_BIT    PINL7

#elif SOFTRX == A0
#define SOFTRX_NAME "A0"
#define UART_PIN       PINA
#define UART_RX_BIT    PINA0
#elif SOFTRX == A1
#define SOFTRX_NAME "A1"
#define UART_PIN       PINA
#define UART_RX_BIT    PINA1
#elif SOFTRX == A2
#define SOFTRX_NAME "A2"
#define UART_PIN       PINA
#define UART_RX_BIT    PINA2
#elif SOFTRX == A3
#define SOFTRX_NAME "A3"
#define UART_PIN       PINA
#define UART_RX_BIT    PINA3
#elif SOFTRX == A4
#define SOFTRX_NAME "A4"
#define UART_PIN       PINA
#define UART_RX_BIT    PINA4
#elif SOFTRX == A5
#define SOFTRX_NAME "A5"
#define UART_PIN       PINA
#define UART_RX_BIT    PINA5
#elif SOFTRX == A6
#define SOFTRX_NAME "A6"
#define UART_PIN       PINA
#define UART_RX_BIT    PINA6
#elif SOFTRX == A7
#define SOFTRX_NAME "A7"
#define UART_PIN       PINA
#define UART_RX_BIT    PINA7
#else
#error -------------------------------------------
#error Unrecognized SOFTRX name.  Should be like "B5"
#error -------------------------------------------
#endif

