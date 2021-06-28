#ifdef __AVR_XMEGA__
#define _mkPort(a) V##a
#else
#define _mkPort(a) a
#endif

#define PIN_INVERTED 0x8000
#define A0_INV (PIN_INVERTED + A0)
#define A1_INV (PIN_INVERTED + A1)
#define A2_INV (PIN_INVERTED + A2)
#define A3_INV (PIN_INVERTED + A3)
#define A4_INV (PIN_INVERTED + A4)
#define A5_INV (PIN_INVERTED + A5)
#define A6_INV (PIN_INVERTED + A6)
#define A7_INV (PIN_INVERTED + A7)
#define B0_INV (PIN_INVERTED + B0)
#define B1_INV (PIN_INVERTED + B1)
#define B2_INV (PIN_INVERTED + B2)
#define B3_INV (PIN_INVERTED + B3)
#define B4_INV (PIN_INVERTED + B4)
#define B5_INV (PIN_INVERTED + B5)
#define B6_INV (PIN_INVERTED + B6)
#define B7_INV (PIN_INVERTED + B7)
#define C0_INV (PIN_INVERTED + C0)
#define C1_INV (PIN_INVERTED + C1)
#define C2_INV (PIN_INVERTED + C2)
#define C3_INV (PIN_INVERTED + C3)
#define C4_INV (PIN_INVERTED + C4)
#define C5_INV (PIN_INVERTED + C5)
#define C6_INV (PIN_INVERTED + C6)
#define C7_INV (PIN_INVERTED + C7)
#define D0_INV (PIN_INVERTED + D0)
#define D1_INV (PIN_INVERTED + D1)
#define D2_INV (PIN_INVERTED + D2)
#define D3_INV (PIN_INVERTED + D3)
#define D4_INV (PIN_INVERTED + D4)
#define D5_INV (PIN_INVERTED + D5)
#define D6_INV (PIN_INVERTED + D6)
#define D7_INV (PIN_INVERTED + D7)
#define E0_INV (PIN_INVERTED + E0)
#define E1_INV (PIN_INVERTED + E1)
#define E2_INV (PIN_INVERTED + E2)
#define E3_INV (PIN_INVERTED + E3)
#define E4_INV (PIN_INVERTED + E4)
#define E5_INV (PIN_INVERTED + E5)
#define E6_INV (PIN_INVERTED + E6)
#define E7_INV (PIN_INVERTED + E7)
#define F0_INV (PIN_INVERTED + F0)
#define F1_INV (PIN_INVERTED + F1)
#define F2_INV (PIN_INVERTED + F2)
#define F3_INV (PIN_INVERTED + F3)
#define F4_INV (PIN_INVERTED + F4)
#define F5_INV (PIN_INVERTED + F5)
#define F6_INV (PIN_INVERTED + F6)
#define F7_INV (PIN_INVERTED + F7)
#define G0_INV (PIN_INVERTED + G0)
#define G1_INV (PIN_INVERTED + G1)
#define G2_INV (PIN_INVERTED + G2)
#define G3_INV (PIN_INVERTED + G3)
#define G4_INV (PIN_INVERTED + G4)
#define G5_INV (PIN_INVERTED + G5)
#define G6_INV (PIN_INVERTED + G6)
#define G7_INV (PIN_INVERTED + G7)
#define H0_INV (PIN_INVERTED + H0)
#define H1_INV (PIN_INVERTED + H1)
#define H2_INV (PIN_INVERTED + H2)
#define H3_INV (PIN_INVERTED + H3)
#define H4_INV (PIN_INVERTED + H4)
#define H5_INV (PIN_INVERTED + H5)
#define H6_INV (PIN_INVERTED + H6)
#define H7_INV (PIN_INVERTED + H7)
#define J0_INV (PIN_INVERTED + J0)
#define J1_INV (PIN_INVERTED + J1)
#define J2_INV (PIN_INVERTED + J2)
#define J3_INV (PIN_INVERTED + J3)
#define J4_INV (PIN_INVERTED + J4)
#define J5_INV (PIN_INVERTED + J5)
#define J6_INV (PIN_INVERTED + J6)
#define J7_INV (PIN_INVERTED + J7)
#define K0_INV (PIN_INVERTED + K0)
#define K1_INV (PIN_INVERTED + K1)
#define K2_INV (PIN_INVERTED + K2)
#define K3_INV (PIN_INVERTED + K3)
#define K4_INV (PIN_INVERTED + K4)
#define K5_INV (PIN_INVERTED + K5)
#define K6_INV (PIN_INVERTED + K6)
#define K7_INV (PIN_INVERTED + K7)
#define L0_INV (PIN_INVERTED + L0)
#define L1_INV (PIN_INVERTED + L1)
#define L2_INV (PIN_INVERTED + L2)
#define L3_INV (PIN_INVERTED + L3)
#define L4_INV (PIN_INVERTED + L4)
#define L5_INV (PIN_INVERTED + L5)
#define L6_INV (PIN_INVERTED + L6)
#define L7_INV (PIN_INVERTED + L7)

#if RS485 == B0
#define RS485_NAME "B0"
#define RS485_DDR     DDRB
#define RS485_PORT    _mkPort(PORTB)
#define RS485_PIN     PINB
#define RS485_BIT     PIN0
#elif RS485 == B1
#define RS485_NAME "B1"
#define RS485_DDR     DDRB
#define RS485_PORT    _mkPort(PORTB)
#define RS485_PIN     PINB
#define RS485_BIT     PIN1
#elif RS485 == B2
#define RS485_NAME "B2"
#define RS485_DDR     DDRB
#define RS485_PORT    _mkPort(PORTB)
#define RS485_PIN     PINB
#define RS485_BIT     PIN2
#elif RS485 == B3
#define RS485_NAME "B3"
#define RS485_DDR     DDRB
#define RS485_PORT    _mkPort(PORTB)
#define RS485_PIN     PINB
#define RS485_BIT     PIN3
#elif RS485 == B4
#define RS485_NAME "B4"
#define RS485_DDR     DDRB
#define RS485_PORT    _mkPort(PORTB)
#define RS485_PIN     PINB
#define RS485_BIT     PIN4
#elif RS485 == B5
#define RS485_NAME "B5"
#define RS485_DDR     DDRB
#define RS485_PORT    _mkPort(PORTB)
#define RS485_PIN     PINB
#define RS485_BIT     PIN5
#elif RS485 == B6
#define RS485_NAME "B6"
#define RS485_DDR     DDRB
#define RS485_PORT    _mkPort(PORTB)
#define RS485_PIN     PINB
#define RS485_BIT     PIN6
#elif RS485 == B7
#define RS485_NAME "B7"
#define RS485_DDR     DDRB
#define RS485_PORT    _mkPort(PORTB)
#define RS485_PIN     PINB
#define RS485_BIT     PIN7

#elif RS485 == C0
#define RS485_NAME "C0"
#define RS485_DDR     DDRC
#define RS485_PORT    _mkPort(PORTC)
#define RS485_PIN     PINC
#define RS485_BIT     PIN0
#elif RS485 == C1
#define RS485_NAME "C1"
#define RS485_DDR     DDRC
#define RS485_PORT    _mkPort(PORTC)
#define RS485_PIN     PINC
#define RS485_BIT     PIN1
#elif RS485 == C2
#define RS485_NAME "C2"
#define RS485_DDR     DDRC
#define RS485_PORT    _mkPort(PORTC)
#define RS485_PIN     PINC
#define RS485_BIT     PIN2
#elif RS485 == C3
#define RS485_NAME "C3"
#define RS485_DDR     DDRC
#define RS485_PORT    _mkPort(PORTC)
#define RS485_PIN     PINC
#define RS485_BIT     PIN3
#elif RS485 == C4
#define RS485_NAME "C4"
#define RS485_DDR     DDRC
#define RS485_PORT    _mkPort(PORTC)
#define RS485_PIN     PINC
#define RS485_BIT     PIN4
#elif RS485 == C5
#define RS485_NAME "C5"
#define RS485_DDR     DDRC
#define RS485_PORT    _mkPort(PORTC)
#define RS485_PIN     PINC
#define RS485_BIT     PIN5
#elif RS485 == C6
#define RS485_NAME "C6"
#define RS485_DDR     DDRC
#define RS485_PORT    _mkPort(PORTC)
#define RS485_PIN     PINC
#define RS485_BIT     PIN6
#elif RS485 == C7
#define RS485_NAME "C7"
#define RS485_DDR     DDRC
#define RS485_PORT    _mkPort(PORTC)
#define RS485_PIN     PINC
#define RS485_BIT     PIN7

#elif RS485 == D0
#define RS485_NAME "D0"
#define RS485_DDR     DDRD
#define RS485_PORT    _mkPort(PORTD)
#define RS485_PIN     PIND
#define RS485_BIT     PIN0
#elif RS485 == D1
#define RS485_NAME "D1"
#define RS485_DDR     DDRD
#define RS485_PORT    _mkPort(PORTD)
#define RS485_PIN     PIND
#define RS485_BIT     PIN1
#elif RS485 == D2
#define RS485_NAME "D2"
#define RS485_DDR     DDRD
#define RS485_PORT    _mkPort(PORTD)
#define RS485_PIN     PIND
#define RS485_BIT     PIN2
#elif RS485 == D3
#define RS485_NAME "D3"
#define RS485_DDR     DDRD
#define RS485_PORT    _mkPort(PORTD)
#define RS485_PIN     PIND
#define RS485_BIT     PIN3
#elif RS485 == D4
#define RS485_NAME "D4"
#define RS485_DDR     DDRD
#define RS485_PORT    _mkPort(PORTD)
#define RS485_PIN     PIND
#define RS485_BIT     PIN4
#elif RS485 == D5
#define RS485_NAME "D5"
#define RS485_DDR     DDRD
#define RS485_PORT    _mkPort(PORTD)
#define RS485_PIN     PIND
#define RS485_BIT     PIN5
#elif RS485 == D6
#define RS485_NAME "D6"
#define RS485_DDR     DDRD
#define RS485_PORT    _mkPort(PORTD)
#define RS485_PIN     PIND
#define RS485_BIT     PIN6
#elif RS485 == D7
#define RS485_NAME "D7"
#define RS485_DDR     DDRD
#define RS485_PORT    _mkPort(PORTD)
#define RS485_PIN     PIND
#define RS485_BIT     PIN7

#elif RS485 == E0
#define RS485_NAME "E0"
#define RS485_DDR     DDRE
#define RS485_PORT    _mkPort(PORTE)
#define RS485_PIN     PINE
#define RS485_BIT     PIN0
#elif RS485 == E1
#define RS485_NAME "E1"
#define RS485_DDR     DDRE
#define RS485_PORT    _mkPort(PORTE)
#define RS485_PIN     PINE
#define RS485_BIT     PIN1
#elif RS485 == E2
#define RS485_NAME "E2"
#define RS485_DDR     DDRE
#define RS485_PORT    _mkPort(PORTE)
#define RS485_PIN     PINE
#define RS485_BIT     PIN2
#elif RS485 == E3
#define RS485_NAME "E3"
#define RS485_DDR     DDRE
#define RS485_PORT    _mkPort(PORTE)
#define RS485_PIN     PINE
#define RS485_BIT     PIN3
#elif RS485 == E4
#define RS485_NAME "E4"
#define RS485_DDR     DDRE
#define RS485_PORT    _mkPort(PORTE)
#define RS485_PIN     PINE
#define RS485_BIT     PIN4
#elif RS485 == E5
#define RS485_NAME "E5"
#define RS485_DDR     DDRE
#define RS485_PORT    _mkPort(PORTE)
#define RS485_PIN     PINE
#define RS485_BIT     PIN5
#elif RS485 == E6
#define RS485_NAME "E6"
#define RS485_DDR     DDRE
#define RS485_PORT    _mkPort(PORTE)
#define RS485_PIN     PINE
#define RS485_BIT     PIN6
#elif RS485 == E7
#define RS485_NAME "E7"
#define RS485_DDR     DDRE
#define RS485_PORT    _mkPort(PORTE)
#define RS485_PIN     PINE
#define RS485_BIT     PIN7

#elif RS485 == F0
#define RS485_NAME "F0"
#define RS485_DDR     DDRF
#define RS485_PORT    _mkPort(PORTF)
#define RS485_PIN     PINF
#define RS485_BIT     PIN0
#elif RS485 == F1
#define RS485_NAME "F1"
#define RS485_DDR     DDRF
#define RS485_PORT    _mkPort(PORTF)
#define RS485_PIN     PINF
#define RS485_BIT     PIN1
#elif RS485 == F2
#define RS485_NAME "F2"
#define RS485_DDR     DDRF
#define RS485_PORT    _mkPort(PORTF)
#define RS485_PIN     PINF
#define RS485_BIT     PIN2
#elif RS485 == F3
#define RS485_NAME "F3"
#define RS485_DDR     DDRF
#define RS485_PORT    _mkPort(PORTF)
#define RS485_PIN     PINF
#define RS485_BIT     PIN3
#elif RS485 == F4
#define RS485_NAME "F4"
#define RS485_DDR     DDRF
#define RS485_PORT    _mkPort(PORTF)
#define RS485_PIN     PINF
#define RS485_BIT     PIN4
#elif RS485 == F5
#define RS485_NAME "F5"
#define RS485_DDR     DDRF
#define RS485_PORT    _mkPort(PORTF)
#define RS485_PIN     PINF
#define RS485_BIT     PIN5
#elif RS485 == F6
#define RS485_NAME "F6"
#define RS485_DDR     DDRF
#define RS485_PORT    _mkPort(PORTF)
#define RS485_PIN     PINF
#define RS485_BIT     PIN6
#elif RS485 == F7
#define RS485_NAME "F7"
#define RS485_DDR     DDRF
#define RS485_PORT    _mkPort(PORTF)
#define RS485_PIN     PINF
#define RS485_BIT     PIN7

#elif RS485 == G0
#define RS485_NAME "G0"
#define RS485_DDR     DDRG
#define RS485_PORT    _mkPort(PORTG)
#define RS485_PIN     PING
#define RS485_BIT     PIN0
#elif RS485 == G1
#define RS485_NAME "G1"
#define RS485_DDR     DDRG
#define RS485_PORT    _mkPort(PORTG)
#define RS485_PIN     PING
#define RS485_BIT     PIN1
#elif RS485 == G2
#define RS485_NAME "G2"
#define RS485_DDR     DDRG
#define RS485_PORT    _mkPort(PORTG)
#define RS485_PIN     PING
#define RS485_BIT     PIN2
#elif RS485 == G3
#define RS485_NAME "G3"
#define RS485_DDR     DDRG
#define RS485_PORT    _mkPort(PORTG)
#define RS485_PIN     PING
#define RS485_BIT     PIN3
#elif RS485 == G4
#define RS485_NAME "G4"
#define RS485_DDR     DDRG
#define RS485_PORT    _mkPort(PORTG)
#define RS485_PIN     PING
#define RS485_BIT     PIN4
#elif RS485 == G5
#define RS485_NAME "G5"
#define RS485_DDR     DDRG
#define RS485_PORT    _mkPort(PORTG)
#define RS485_PIN     PING
#define RS485_BIT     PIN5
#elif RS485 == G6
#define RS485_NAME "G6"
#define RS485_DDR     DDRG
#define RS485_PORT    _mkPort(PORTG)
#define RS485_PIN     PING
#define RS485_BIT     PIN6
#elif RS485 == G7
#define RS485_NAME "G7"
#define RS485_DDR     DDRG
#define RS485_PORT    _mkPort(PORTG)
#define RS485_PIN     PING
#define RS485_BIT     PIN7

#elif RS485 == H0
#define RS485_NAME "H0"
#define RS485_DDR     DDRH
#define RS485_PORT    _mkPort(PORTH)
#define RS485_PIN     PINH
#define RS485_BIT     PIN0
#elif RS485 == H1
#define RS485_NAME "H1"
#define RS485_DDR     DDRH
#define RS485_PORT    _mkPort(PORTH)
#define RS485_PIN     PINH
#define RS485_BIT     PIN1
#elif RS485 == H2
#define RS485_NAME "H2"
#define RS485_DDR     DDRH
#define RS485_PORT    _mkPort(PORTH)
#define RS485_PIN     PINH
#define RS485_BIT     PIN2
#elif RS485 == H3
#define RS485_NAME "H3"
#define RS485_DDR     DDRH
#define RS485_PORT    _mkPort(PORTH)
#define RS485_PIN     PINH
#define RS485_BIT     PIN3
#elif RS485 == H4
#define RS485_NAME "H4"
#define RS485_DDR     DDRH
#define RS485_PORT    _mkPort(PORTH)
#define RS485_PIN     PINH
#define RS485_BIT     PIN4
#elif RS485 == H5
#define RS485_NAME "H5"
#define RS485_DDR     DDRH
#define RS485_PORT    _mkPort(PORTH)
#define RS485_PIN     PINH
#define RS485_BIT     PIN5
#elif RS485 == H6
#define RS485_NAME "H6"
#define RS485_DDR     DDRH
#define RS485_PORT    _mkPort(PORTH)
#define RS485_PIN     PINH
#define RS485_BIT     PIN6
#elif RS485 == H7
#define RS485_NAME "H7"
#define RS485_DDR     DDRH
#define RS485_PORT    _mkPort(PORTH)
#define RS485_PIN     PINH
#define RS485_BIT     PIN7

#elif RS485 == J0
#define RS485_NAME "J0"
#define RS485_DDR     DDRJ
#define RS485_PORT    _mkPort(PORTJ)
#define RS485_PIN     PINJ
#define RS485_BIT     PIN0
#elif RS485 == J1
#define RS485_NAME "J1"
#define RS485_DDR     DDRJ
#define RS485_PORT    _mkPort(PORTJ)
#define RS485_PIN     PINJ
#define RS485_BIT     PIN1
#elif RS485 == J2
#define RS485_NAME "J2"
#define RS485_DDR     DDRJ
#define RS485_PORT    _mkPort(PORTJ)
#define RS485_PIN     PINJ
#define RS485_BIT     PIN2
#elif RS485 == J3
#define RS485_NAME "J3"
#define RS485_DDR     DDRJ
#define RS485_PORT    _mkPort(PORTJ)
#define RS485_PIN     PINJ
#define RS485_BIT     PIN3
#elif RS485 == J4
#define RS485_NAME "J4"
#define RS485_DDR     DDRJ
#define RS485_PORT    _mkPort(PORTJ)
#define RS485_PIN     PINJ
#define RS485_BIT     PIN4
#elif RS485 == J5
#define RS485_NAME "J5"
#define RS485_DDR     DDRJ
#define RS485_PORT    _mkPort(PORTJ)
#define RS485_PIN     PINJ
#define RS485_BIT     PIN5
#elif RS485 == J6
#define RS485_NAME "J6"
#define RS485_DDR     DDRJ
#define RS485_PORT    _mkPort(PORTJ)
#define RS485_PIN     PINJ
#define RS485_BIT     PIN6
#elif RS485 == J7
#define RS485_NAME "J7"
#define RS485_DDR     DDRJ
#define RS485_PORT    _mkPort(PORTJ)
#define RS485_PIN     PINJ
#define RS485_BIT     PIN7

#elif RS485 == K0
#define RS485_NAME "K0"
#define RS485_DDR     DDRK
#define RS485_PORT    _mkPort(PORTK)
#define RS485_PIN     PINK
#define RS485_BIT     PIN0
#elif RS485 == K1
#define RS485_NAME "K1"
#define RS485_DDR     DDRK
#define RS485_PORT    _mkPort(PORTK)
#define RS485_PIN     PINK
#define RS485_BIT     PIN1
#elif RS485 == K2
#define RS485_NAME "K2"
#define RS485_DDR     DDRK
#define RS485_PORT    _mkPort(PORTK)
#define RS485_PIN     PINK
#define RS485_BIT     PIN2
#elif RS485 == K3
#define RS485_NAME "K3"
#define RS485_DDR     DDRK
#define RS485_PORT    _mkPort(PORTK)
#define RS485_PIN     PINK
#define RS485_BIT     PIN3
#elif RS485 == K4
#define RS485_NAME "K4"
#define RS485_DDR     DDRK
#define RS485_PORT    _mkPort(PORTK)
#define RS485_PIN     PINK
#define RS485_BIT     PIN4
#elif RS485 == K5
#define RS485_NAME "K5"
#define RS485_DDR     DDRK
#define RS485_PORT    _mkPort(PORTK)
#define RS485_PIN     PINK
#define RS485_BIT     PIN5
#elif RS485 == K6
#define RS485_NAME "K6"
#define RS485_DDR     DDRK
#define RS485_PORT    _mkPort(PORTK)
#define RS485_PIN     PINK
#define RS485_BIT     PIN6
#elif RS485 == K7
#define RS485_NAME "K7"
#define RS485_DDR     DDRK
#define RS485_PORT    _mkPort(PORTK)
#define RS485_PIN     PINK
#define RS485_BIT     PIN7

#elif RS485 == L0
#define RS485_NAME "L0"
#define RS485_DDR     DDRL
#define RS485_PORT    _mkPort(PORTL)
#define RS485_PIN     PINL
#define RS485_BIT     PIN0
#elif RS485 == L1
#define RS485_NAME "L1"
#define RS485_DDR     DDRL
#define RS485_PORT    _mkPort(PORTL)
#define RS485_PIN     PINL
#define RS485_BIT     PIN1
#elif RS485 == L2
#define RS485_NAME "L2"
#define RS485_DDR     DDRL
#define RS485_PORT    _mkPort(PORTL)
#define RS485_PIN     PINL
#define RS485_BIT     PIN2
#elif RS485 == L3
#define RS485_NAME "L3"
#define RS485_DDR     DDRL
#define RS485_PORT    _mkPort(PORTL)
#define RS485_PIN     PINL
#define RS485_BIT     PIN3
#elif RS485 == L4
#define RS485_NAME "L4"
#define RS485_DDR     DDRL
#define RS485_PORT    _mkPort(PORTL)
#define RS485_PIN     PINL
#define RS485_BIT     PIN4
#elif RS485 == L5
#define RS485_NAME "L5"
#define RS485_DDR     DDRL
#define RS485_PORT    _mkPort(PORTL)
#define RS485_PIN     PINL
#define RS485_BIT     PIN5
#elif RS485 == L6
#define RS485_NAME "L6"
#define RS485_DDR     DDRL
#define RS485_PORT    _mkPort(PORTL)
#define RS485_PIN     PINL
#define RS485_BIT     PIN6
#elif RS485 == L7
#define RS485_NAME "L7"
#define RS485_DDR     DDRL
#define RS485_PORT    _mkPort(PORTL)
#define RS485_PIN     PINL
#define RS485_BIT     PIN7

#elif RS485 == A0
#define RS485_NAME "A0"
#define RS485_DDR     DDRA
#define RS485_PORT    _mkPort(PORTA)
#define RS485_PIN     PINA
#define RS485_BIT     PIN0
#elif RS485 == A1
#define RS485_NAME "A1"
#define RS485_DDR     DDRA
#define RS485_PORT    _mkPort(PORTA)
#define RS485_PIN     PINA
#define RS485_BIT     PIN1
#elif RS485 == A2
#define RS485_NAME "A2"
#define RS485_DDR     DDRA
#define RS485_PORT    _mkPort(PORTA)
#define RS485_PIN     PINA
#define RS485_BIT     PIN2
#elif RS485 == A3
#define RS485_NAME "A3"
#define RS485_DDR     DDRA
#define RS485_PORT    _mkPort(PORTA)
#define RS485_PIN     PINA
#define RS485_BIT     PIN3
#elif RS485 == A4
#define RS485_NAME "A4"
#define RS485_DDR     DDRA
#define RS485_PORT    _mkPort(PORTA)
#define RS485_PIN     PINA
#define RS485_BIT     PIN4
#elif RS485 == A5
#define RS485_NAME "A5"
#define RS485_DDR     DDRA
#define RS485_PORT    _mkPort(PORTA)
#define RS485_PIN     PINA
#define RS485_BIT     PIN5
#elif RS485 == A6
#define RS485_NAME "A6"
#define RS485_DDR     DDRA
#define RS485_PORT    _mkPort(PORTA)
#define RS485_PIN     PINA
#define RS485_BIT     PIN6
#elif RS485 == A7
#define RS485_NAME "A7"
#define RS485_DDR     DDRA
#define RS485_PORT    _mkPort(PORTA)
#define RS485_PIN     PINA
#define RS485_BIT     PIN7
#elif RS485 == B0_INV
#define RS485_NAME "B0_INV"
#define RS485_INVERT
#define RS485_DDR     DDRB
#define RS485_PORT    _mkPort(PORTB)
#define RS485_PIN     PINB
#define RS485_BIT     PIN0
#elif RS485 == B1_INV
#define RS485_NAME "B1_INV"
#define RS485_INVERT
#define RS485_DDR     DDRB
#define RS485_PORT    _mkPort(PORTB)
#define RS485_PIN     PINB
#define RS485_BIT     PIN1
#elif RS485 == B2_INV
#define RS485_NAME "B2_INV"
#define RS485_INVERT
#define RS485_DDR     DDRB
#define RS485_PORT    _mkPort(PORTB)
#define RS485_PIN     PINB
#define RS485_BIT     PIN2
#elif RS485 == B3_INV
#define RS485_NAME "B3_INV"
#define RS485_INVERT
#define RS485_DDR     DDRB
#define RS485_PORT    _mkPort(PORTB)
#define RS485_PIN     PINB
#define RS485_BIT     PIN3
#elif RS485 == B4_INV
#define RS485_NAME "B4_INV"
#define RS485_INVERT
#define RS485_DDR     DDRB
#define RS485_PORT    _mkPort(PORTB)
#define RS485_PIN     PINB
#define RS485_BIT     PIN4
#elif RS485 == B5_INV
#define RS485_NAME "B5_INV"
#define RS485_INVERT
#define RS485_DDR     DDRB
#define RS485_PORT    _mkPort(PORTB)
#define RS485_PIN     PINB
#define RS485_BIT     PIN5
#elif RS485 == B6_INV
#define RS485_NAME "B6_INV"
#define RS485_INVERT
#define RS485_DDR     DDRB
#define RS485_PORT    _mkPort(PORTB)
#define RS485_PIN     PINB
#define RS485_BIT     PIN6
#elif RS485 == B7_INV
#define RS485_NAME "B7_INV"
#define RS485_INVERT
#define RS485_DDR     DDRB
#define RS485_PORT    _mkPort(PORTB)
#define RS485_PIN     PINB
#define RS485_BIT     PIN7

#elif RS485 == C0_INV
#define RS485_NAME "C0_INV"
#define RS485_INVERT
#define RS485_DDR     DDRC
#define RS485_PORT    _mkPort(PORTC)
#define RS485_PIN     PINC
#define RS485_BIT     PIN0
#elif RS485 == C1_INV
#define RS485_NAME "C1_INV"
#define RS485_INVERT
#define RS485_DDR     DDRC
#define RS485_PORT    _mkPort(PORTC)
#define RS485_PIN     PINC
#define RS485_BIT     PIN1
#elif RS485 == C2_INV
#define RS485_NAME "C2_INV"
#define RS485_INVERT
#define RS485_DDR     DDRC
#define RS485_PORT    _mkPort(PORTC)
#define RS485_PIN     PINC
#define RS485_BIT     PIN2
#elif RS485 == C3_INV
#define RS485_NAME "C3_INV"
#define RS485_INVERT
#define RS485_DDR     DDRC
#define RS485_PORT    _mkPort(PORTC)
#define RS485_PIN     PINC
#define RS485_BIT     PIN3
#elif RS485 == C4_INV
#define RS485_NAME "C4_INV"
#define RS485_INVERT
#define RS485_DDR     DDRC
#define RS485_PORT    _mkPort(PORTC)
#define RS485_PIN     PINC
#define RS485_BIT     PIN4
#elif RS485 == C5_INV
#define RS485_NAME "C5_INV"
#define RS485_INVERT
#define RS485_DDR     DDRC
#define RS485_PORT    _mkPort(PORTC)
#define RS485_PIN     PINC
#define RS485_BIT     PIN5
#elif RS485 == C6_INV
#define RS485_NAME "C6_INV"
#define RS485_INVERT
#define RS485_DDR     DDRC
#define RS485_PORT    _mkPort(PORTC)
#define RS485_PIN     PINC
#define RS485_BIT     PIN6
#elif RS485 == C7_INV
#define RS485_NAME "C7_INV"
#define RS485_INVERT
#define RS485_DDR     DDRC
#define RS485_PORT    _mkPort(PORTC)
#define RS485_PIN     PINC
#define RS485_BIT     PIN7

#elif RS485 == D0_INV
#define RS485_NAME "D0_INV"
#define RS485_INVERT
#define RS485_DDR     DDRD
#define RS485_PORT    _mkPort(PORTD)
#define RS485_PIN     PIND
#define RS485_BIT     PIN0
#elif RS485 == D1_INV
#define RS485_NAME "D1_INV"
#define RS485_INVERT
#define RS485_DDR     DDRD
#define RS485_PORT    _mkPort(PORTD)
#define RS485_PIN     PIND
#define RS485_BIT     PIN1
#elif RS485 == D2_INV
#define RS485_NAME "D2_INV"
#define RS485_INVERT
#define RS485_DDR     DDRD
#define RS485_PORT    _mkPort(PORTD)
#define RS485_PIN     PIND
#define RS485_BIT     PIN2
#elif RS485 == D3_INV
#define RS485_NAME "D3_INV"
#define RS485_INVERT
#define RS485_DDR     DDRD
#define RS485_PORT    _mkPort(PORTD)
#define RS485_PIN     PIND
#define RS485_BIT     PIN3
#elif RS485 == D4_INV
#define RS485_NAME "D4_INV"
#define RS485_INVERT
#define RS485_DDR     DDRD
#define RS485_PORT    _mkPort(PORTD)
#define RS485_PIN     PIND
#define RS485_BIT     PIN4
#elif RS485 == D5_INV
#define RS485_NAME "D5_INV"
#define RS485_INVERT
#define RS485_DDR     DDRD
#define RS485_PORT    _mkPort(PORTD)
#define RS485_PIN     PIND
#define RS485_BIT     PIN5
#elif RS485 == D6_INV
#define RS485_NAME "D6_INV"
#define RS485_INVERT
#define RS485_DDR     DDRD
#define RS485_PORT    _mkPort(PORTD)
#define RS485_PIN     PIND
#define RS485_BIT     PIN6
#elif RS485 == D7_INV
#define RS485_NAME "D7_INV"
#define RS485_INVERT
#define RS485_DDR     DDRD
#define RS485_PORT    _mkPort(PORTD)
#define RS485_PIN     PIND
#define RS485_BIT     PIN7

#elif RS485 == E0_INV
#define RS485_NAME "E0_INV"
#define RS485_INVERT
#define RS485_DDR     DDRE
#define RS485_PORT    _mkPort(PORTE)
#define RS485_PIN     PINE
#define RS485_BIT     PIN0
#elif RS485 == E1_INV
#define RS485_NAME "E1_INV"
#define RS485_INVERT
#define RS485_DDR     DDRE
#define RS485_PORT    _mkPort(PORTE)
#define RS485_PIN     PINE
#define RS485_BIT     PIN1
#elif RS485 == E2_INV
#define RS485_NAME "E2_INV"
#define RS485_INVERT
#define RS485_DDR     DDRE
#define RS485_PORT    _mkPort(PORTE)
#define RS485_PIN     PINE
#define RS485_BIT     PIN2
#elif RS485 == E3_INV
#define RS485_NAME "E3_INV"
#define RS485_INVERT
#define RS485_DDR     DDRE
#define RS485_PORT    _mkPort(PORTE)
#define RS485_PIN     PINE
#define RS485_BIT     PIN3
#elif RS485 == E4_INV
#define RS485_NAME "E4_INV"
#define RS485_INVERT
#define RS485_DDR     DDRE
#define RS485_PORT    _mkPort(PORTE)
#define RS485_PIN     PINE
#define RS485_BIT     PIN4
#elif RS485 == E5_INV
#define RS485_NAME "E5_INV"
#define RS485_INVERT
#define RS485_DDR     DDRE
#define RS485_PORT    _mkPort(PORTE)
#define RS485_PIN     PINE
#define RS485_BIT     PIN5
#elif RS485 == E6_INV
#define RS485_NAME "E6_INV"
#define RS485_INVERT
#define RS485_DDR     DDRE
#define RS485_PORT    _mkPort(PORTE)
#define RS485_PIN     PINE
#define RS485_BIT     PIN6
#elif RS485 == E7_INV
#define RS485_NAME "E7_INV"
#define RS485_INVERT
#define RS485_DDR     DDRE
#define RS485_PORT    _mkPort(PORTE)
#define RS485_PIN     PINE
#define RS485_BIT     PIN7

#elif RS485 == F0_INV
#define RS485_NAME "F0_INV"
#define RS485_INVERT
#define RS485_DDR     DDRF
#define RS485_PORT    _mkPort(PORTF)
#define RS485_PIN     PINF
#define RS485_BIT     PIN0
#elif RS485 == F1_INV
#define RS485_NAME "F1_INV"
#define RS485_INVERT
#define RS485_DDR     DDRF
#define RS485_PORT    _mkPort(PORTF)
#define RS485_PIN     PINF
#define RS485_BIT     PIN1
#elif RS485 == F2_INV
#define RS485_NAME "F2_INV"
#define RS485_INVERT
#define RS485_DDR     DDRF
#define RS485_PORT    _mkPort(PORTF)
#define RS485_PIN     PINF
#define RS485_BIT     PIN2
#elif RS485 == F3_INV
#define RS485_NAME "F3_INV"
#define RS485_INVERT
#define RS485_DDR     DDRF
#define RS485_PORT    _mkPort(PORTF)
#define RS485_PIN     PINF
#define RS485_BIT     PIN3
#elif RS485 == F4_INV
#define RS485_NAME "F4_INV"
#define RS485_INVERT
#define RS485_DDR     DDRF
#define RS485_PORT    _mkPort(PORTF)
#define RS485_PIN     PINF
#define RS485_BIT     PIN4
#elif RS485 == F5_INV
#define RS485_NAME "F5_INV"
#define RS485_INVERT
#define RS485_DDR     DDRF
#define RS485_PORT    _mkPort(PORTF)
#define RS485_PIN     PINF
#define RS485_BIT     PIN5
#elif RS485 == F6_INV
#define RS485_NAME "F6_INV"
#define RS485_INVERT
#define RS485_DDR     DDRF
#define RS485_PORT    _mkPort(PORTF)
#define RS485_PIN     PINF
#define RS485_BIT     PIN6
#elif RS485 == F7_INV
#define RS485_NAME "F7_INV"
#define RS485_INVERT
#define RS485_DDR     DDRF
#define RS485_PORT    _mkPort(PORTF)
#define RS485_PIN     PINF
#define RS485_BIT     PIN7

#elif RS485 == G0_INV
#define RS485_NAME "G0_INV"
#define RS485_INVERT
#define RS485_DDR     DDRG
#define RS485_PORT    _mkPort(PORTG)
#define RS485_PIN     PING
#define RS485_BIT     PIN0
#elif RS485 == G1_INV
#define RS485_NAME "G1_INV"
#define RS485_INVERT
#define RS485_DDR     DDRG
#define RS485_PORT    _mkPort(PORTG)
#define RS485_PIN     PING
#define RS485_BIT     PIN1
#elif RS485 == G2_INV
#define RS485_NAME "G2_INV"
#define RS485_INVERT
#define RS485_DDR     DDRG
#define RS485_PORT    _mkPort(PORTG)
#define RS485_PIN     PING
#define RS485_BIT     PIN2
#elif RS485 == G3_INV
#define RS485_NAME "G3_INV"
#define RS485_INVERT
#define RS485_DDR     DDRG
#define RS485_PORT    _mkPort(PORTG)
#define RS485_PIN     PING
#define RS485_BIT     PIN3
#elif RS485 == G4_INV
#define RS485_NAME "G4_INV"
#define RS485_INVERT
#define RS485_DDR     DDRG
#define RS485_PORT    _mkPort(PORTG)
#define RS485_PIN     PING
#define RS485_BIT     PIN4
#elif RS485 == G5_INV
#define RS485_NAME "G5_INV"
#define RS485_INVERT
#define RS485_DDR     DDRG
#define RS485_PORT    _mkPort(PORTG)
#define RS485_PIN     PING
#define RS485_BIT     PIN5
#elif RS485 == G6_INV
#define RS485_NAME "G6_INV"
#define RS485_INVERT
#define RS485_DDR     DDRG
#define RS485_PORT    _mkPort(PORTG)
#define RS485_PIN     PING
#define RS485_BIT     PIN6
#elif RS485 == G7_INV
#define RS485_NAME "G7_INV"
#define RS485_INVERT
#define RS485_DDR     DDRG
#define RS485_PORT    _mkPort(PORTG)
#define RS485_PIN     PING
#define RS485_BIT     PIN7

#elif RS485 == H0_INV
#define RS485_NAME "H0_INV"
#define RS485_INVERT
#define RS485_DDR     DDRH
#define RS485_PORT    _mkPort(PORTH)
#define RS485_PIN     PINH
#define RS485_BIT     PIN0
#elif RS485 == H1_INV
#define RS485_NAME "H1_INV"
#define RS485_INVERT
#define RS485_DDR     DDRH
#define RS485_PORT    _mkPort(PORTH)
#define RS485_PIN     PINH
#define RS485_BIT     PIN1
#elif RS485 == H2_INV
#define RS485_NAME "H2_INV"
#define RS485_INVERT
#define RS485_DDR     DDRH
#define RS485_PORT    _mkPort(PORTH)
#define RS485_PIN     PINH
#define RS485_BIT     PIN2
#elif RS485 == H3_INV
#define RS485_NAME "H3_INV"
#define RS485_INVERT
#define RS485_DDR     DDRH
#define RS485_PORT    _mkPort(PORTH)
#define RS485_PIN     PINH
#define RS485_BIT     PIN3
#elif RS485 == H4_INV
#define RS485_NAME "H4_INV"
#define RS485_INVERT
#define RS485_DDR     DDRH
#define RS485_PORT    _mkPort(PORTH)
#define RS485_PIN     PINH
#define RS485_BIT     PIN4
#elif RS485 == H5_INV
#define RS485_NAME "H5_INV"
#define RS485_INVERT
#define RS485_DDR     DDRH
#define RS485_PORT    _mkPort(PORTH)
#define RS485_PIN     PINH
#define RS485_BIT     PIN5
#elif RS485 == H6_INV
#define RS485_NAME "H6_INV"
#define RS485_INVERT
#define RS485_DDR     DDRH
#define RS485_PORT    _mkPort(PORTH)
#define RS485_PIN     PINH
#define RS485_BIT     PIN6
#elif RS485 == H7_INV
#define RS485_NAME "H7_INV"
#define RS485_INVERT
#define RS485_DDR     DDRH
#define RS485_PORT    _mkPort(PORTH)
#define RS485_PIN     PINH
#define RS485_BIT     PIN7

#elif RS485 == J0_INV
#define RS485_NAME "J0_INV"
#define RS485_INVERT
#define RS485_DDR     DDRJ
#define RS485_PORT    _mkPort(PORTJ)
#define RS485_PIN     PINJ
#define RS485_BIT     PIN0
#elif RS485 == J1_INV
#define RS485_NAME "J1_INV"
#define RS485_INVERT
#define RS485_DDR     DDRJ
#define RS485_PORT    _mkPort(PORTJ)
#define RS485_PIN     PINJ
#define RS485_BIT     PIN1
#elif RS485 == J2_INV
#define RS485_NAME "J2_INV"
#define RS485_INVERT
#define RS485_DDR     DDRJ
#define RS485_PORT    _mkPort(PORTJ)
#define RS485_PIN     PINJ
#define RS485_BIT     PIN2
#elif RS485 == J3_INV
#define RS485_NAME "J3_INV"
#define RS485_INVERT
#define RS485_DDR     DDRJ
#define RS485_PORT    _mkPort(PORTJ)
#define RS485_PIN     PINJ
#define RS485_BIT     PIN3
#elif RS485 == J4_INV
#define RS485_NAME "J4_INV"
#define RS485_INVERT
#define RS485_DDR     DDRJ
#define RS485_PORT    _mkPort(PORTJ)
#define RS485_PIN     PINJ
#define RS485_BIT     PIN4
#elif RS485 == J5_INV
#define RS485_NAME "J5_INV"
#define RS485_INVERT
#define RS485_DDR     DDRJ
#define RS485_PORT    _mkPort(PORTJ)
#define RS485_PIN     PINJ
#define RS485_BIT     PIN5
#elif RS485 == J6_INV
#define RS485_NAME "J6_INV"
#define RS485_INVERT
#define RS485_DDR     DDRJ
#define RS485_PORT    _mkPort(PORTJ)
#define RS485_PIN     PINJ
#define RS485_BIT     PIN6
#elif RS485 == J7_INV
#define RS485_NAME "J7_INV"
#define RS485_INVERT
#define RS485_DDR     DDRJ
#define RS485_PORT    _mkPort(PORTJ)
#define RS485_PIN     PINJ
#define RS485_BIT     PIN7

#elif RS485 == K0_INV
#define RS485_NAME "K0_INV"
#define RS485_INVERT
#define RS485_DDR     DDRK
#define RS485_PORT    _mkPort(PORTK)
#define RS485_PIN     PINK
#define RS485_BIT     PIN0
#elif RS485 == K1_INV
#define RS485_NAME "K1_INV"
#define RS485_INVERT
#define RS485_DDR     DDRK
#define RS485_PORT    _mkPort(PORTK)
#define RS485_PIN     PINK
#define RS485_BIT     PIN1
#elif RS485 == K2_INV
#define RS485_NAME "K2_INV"
#define RS485_INVERT
#define RS485_DDR     DDRK
#define RS485_PORT    _mkPort(PORTK)
#define RS485_PIN     PINK
#define RS485_BIT     PIN2
#elif RS485 == K3_INV
#define RS485_NAME "K3_INV"
#define RS485_INVERT
#define RS485_DDR     DDRK
#define RS485_PORT    _mkPort(PORTK)
#define RS485_PIN     PINK
#define RS485_BIT     PIN3
#elif RS485 == K4_INV
#define RS485_NAME "K4_INV"
#define RS485_INVERT
#define RS485_DDR     DDRK
#define RS485_PORT    _mkPort(PORTK)
#define RS485_PIN     PINK
#define RS485_BIT     PIN4
#elif RS485 == K5_INV
#define RS485_NAME "K5_INV"
#define RS485_INVERT
#define RS485_DDR     DDRK
#define RS485_PORT    _mkPort(PORTK)
#define RS485_PIN     PINK
#define RS485_BIT     PIN5
#elif RS485 == K6_INV
#define RS485_NAME "K6_INV"
#define RS485_INVERT
#define RS485_DDR     DDRK
#define RS485_PORT    _mkPort(PORTK)
#define RS485_PIN     PINK
#define RS485_BIT     PIN6
#elif RS485 == K7_INV
#define RS485_NAME "K7_INV"
#define RS485_INVERT
#define RS485_DDR     DDRK
#define RS485_PORT    _mkPort(PORTK)
#define RS485_PIN     PINK
#define RS485_BIT     PIN7

#elif RS485 == L0_INV
#define RS485_NAME "L0_INV"
#define RS485_INVERT
#define RS485_DDR     DDRL
#define RS485_PORT    _mkPort(PORTL)
#define RS485_PIN     PINL
#define RS485_BIT     PIN0
#elif RS485 == L1_INV
#define RS485_NAME "L1_INV"
#define RS485_INVERT
#define RS485_DDR     DDRL
#define RS485_PORT    _mkPort(PORTL)
#define RS485_PIN     PINL
#define RS485_BIT     PIN1
#elif RS485 == L2_INV
#define RS485_NAME "L2_INV"
#define RS485_INVERT
#define RS485_DDR     DDRL
#define RS485_PORT    _mkPort(PORTL)
#define RS485_PIN     PINL
#define RS485_BIT     PIN2
#elif RS485 == L3_INV
#define RS485_NAME "L3_INV"
#define RS485_INVERT
#define RS485_DDR     DDRL
#define RS485_PORT    _mkPort(PORTL)
#define RS485_PIN     PINL
#define RS485_BIT     PIN3
#elif RS485 == L4_INV
#define RS485_NAME "L4_INV"
#define RS485_INVERT
#define RS485_DDR     DDRL
#define RS485_PORT    _mkPort(PORTL)
#define RS485_PIN     PINL
#define RS485_BIT     PIN4
#elif RS485 == L5_INV
#define RS485_NAME "L5_INV"
#define RS485_INVERT
#define RS485_DDR     DDRL
#define RS485_PORT    _mkPort(PORTL)
#define RS485_PIN     PINL
#define RS485_BIT     PIN5
#elif RS485 == L6_INV
#define RS485_NAME "L6_INV"
#define RS485_INVERT
#define RS485_DDR     DDRL
#define RS485_PORT    _mkPort(PORTL)
#define RS485_PIN     PINL
#define RS485_BIT     PIN6
#elif RS485 == L7_INV
#define RS485_NAME "L7_INV"
#define RS485_INVERT
#define RS485_DDR     DDRL
#define RS485_PORT    _mkPort(PORTL)
#define RS485_PIN     PINL
#define RS485_BIT     PIN7

#elif RS485 == A0_INV
#define RS485_NAME "A0_INV"
#define RS485_INVERT
#define RS485_DDR     DDRA
#define RS485_PORT    _mkPort(PORTA)
#define RS485_PIN     PINA
#define RS485_BIT     PIN0
#elif RS485 == A1_INV
#define RS485_NAME "A1_INV"
#define RS485_INVERT
#define RS485_DDR     DDRA
#define RS485_PORT    _mkPort(PORTA)
#define RS485_PIN     PINA
#define RS485_BIT     PIN1
#elif RS485 == A2_INV
#define RS485_NAME "A2_INV"
#define RS485_INVERT
#define RS485_DDR     DDRA
#define RS485_PORT    _mkPort(PORTA)
#define RS485_PIN     PINA
#define RS485_BIT     PIN2
#elif RS485 == A3_INV
#define RS485_NAME "A3_INV"
#define RS485_INVERT
#define RS485_DDR     DDRA
#define RS485_PORT    _mkPort(PORTA)
#define RS485_PIN     PINA
#define RS485_BIT     PIN3
#elif RS485 == A4_INV
#define RS485_NAME "A4_INV"
#define RS485_INVERT
#define RS485_DDR     DDRA
#define RS485_PORT    _mkPort(PORTA)
#define RS485_PIN     PINA
#define RS485_BIT     PIN4
#elif RS485 == A5_INV
#define RS485_NAME "A5_INV"
#define RS485_INVERT
#define RS485_DDR     DDRA
#define RS485_PORT    _mkPort(PORTA)
#define RS485_PIN     PINA
#define RS485_BIT     PIN5
#elif RS485 == A6_INV
#define RS485_NAME "A6_INV"
#define RS485_INVERT
#define RS485_DDR     DDRA
#define RS485_PORT    _mkPort(PORTA)
#define RS485_PIN     PINA
#define RS485_BIT     PIN6
#elif RS485 == A7_INV
#define RS485_NAME "A7_INV"
#define RS485_INVERT
#define RS485_DDR     DDRA
#define RS485_PORT    _mkPort(PORTA)
#define RS485_PIN     PINA
#define RS485_BIT     PIN7
#else
#error -------------------------------------------
#error Unrecognized RS485 name.  Should be like "B5"
#error or "B5_INV" for active high
#error -------------------------------------------
#endif
