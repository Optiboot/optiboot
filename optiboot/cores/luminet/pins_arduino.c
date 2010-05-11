/*
  pins_arduino.c - pin definitions for the Arduino board
  Part of Arduino / Wiring Lite

  Copyright (c) 2005 David A. Mellis

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General
  Public License along with this library; if not, write to the
  Free Software Foundation, Inc., 59 Temple Place, Suite 330,
  Boston, MA  02111-1307  USA

  $Id: pins_arduino.c 522 2008-11-01 22:06:13Z mellis $
*/

#include <avr/io.h>
#include "wiring_private.h"
#include "pins_arduino.h"

// On the Arduino board, digital pins are also used
// for the analog output (software PWM).  Analog input
// pins are a separate set.





// Luminet board
//                *   *
//                D0  D1
//						VCC PA7 PB2 GND
//             |   |   |   |
//             |   |   |   |
//    VCC---                   ---VCC
//D7  PA3---                   ---PB1  D2
//D6  PA2---                   ---PB0  D3
//    GND---                   ---GND
//             |   |   |   |
//             |   |   |   |
//            VCC PA1 PA0 GND
//                D5  D4
//
// red   led:   PA4 (d8) -- pin available if jumper is not connected
// green led: * PA5 (d9)  -- pin available if jumper is not connected
// blue  led: * PA6 (d10)  -- pin available if jumper is not connected
// reset:       PB3 (d11)
//
// * indicates the PWM pins on the attiny84

#define PA 2
#define PB 3


// these arrays map port names (e.g. port B) to the
// appropriate addresses for various functions (e.g. reading
// and writing)
const uint8_t PROGMEM port_to_mode_PGM[] = {
	NOT_A_PORT,
	NOT_A_PORT,
	&DDRA,
	&DDRB,
};

const uint8_t PROGMEM port_to_output_PGM[] = {
	NOT_A_PORT,
	NOT_A_PORT,
	&PORTA,
	&PORTB,
};

const uint8_t PROGMEM port_to_input_PGM[] = {
	NOT_A_PORT,
	NOT_A_PORT,
	&PINA,
	&PINB,
};

const uint8_t PROGMEM digital_pin_to_port_PGM[] = {
	PA, /* 0 */
	PB,
	PB,
	PB,
	PA,
	PA,
	PA,
	PA,
	PA, /* 8 */
	PA,
	PA,
	PB,
	PA,
	PA,
	PA, /* 14 */
	PA,
	PA,
	PA,
	PA,
	PA,
};

const uint8_t PROGMEM digital_pin_to_bit_mask_PGM[] = {
	_BV(7), /* 0, port D */
	_BV(2),
	_BV(1),
	_BV(0),
	_BV(0),
	_BV(1),
	_BV(2),
	_BV(3),
	_BV(4), /* 8 */
	_BV(5),
	_BV(6),
	_BV(3),
	_BV(4),
	_BV(6),
	_BV(0), /* 14 */
	_BV(1),
	_BV(2),
	_BV(3),
	_BV(4),
	_BV(5),
};

const uint8_t PROGMEM digital_pin_to_timer_PGM[] = {
	NOT_ON_TIMER, /* 0 - port D */
	NOT_ON_TIMER,
	NOT_ON_TIMER,
	NOT_ON_TIMER,
	NOT_ON_TIMER,
	NOT_ON_TIMER,
	NOT_ON_TIMER,
	NOT_ON_TIMER,
	NOT_ON_TIMER, /* 8 - port B */
	NOT_ON_TIMER,
	NOT_ON_TIMER,
	NOT_ON_TIMER,
	NOT_ON_TIMER,
	NOT_ON_TIMER,
	NOT_ON_TIMER, /* 14 - port C */
	NOT_ON_TIMER,
	NOT_ON_TIMER,
	NOT_ON_TIMER,
	NOT_ON_TIMER,
};

