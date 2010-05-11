/*
  wiring_analog.c - analog input and output
  Part of Arduino - http://www.arduino.cc/

  Copyright (c) 2005-2006 David A. Mellis

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

  $Id: wiring.c 248 2007-02-03 15:36:30Z mellis $
*/

#include "bynase.h"

#include "wiring_private.h"
#include "pins_arduino.h"


uint8_t analog_reference = DEFAULT;

void analogReference(uint8_t mode)
{
	// can't actually set the register here because the default setting
	// will connect AVCC and the AREF pin, which would cause a short if
	// there's something connected to AREF.
	analog_reference = mode;
}

/*
long _map(long x, long in_min, long in_max, long out_min, long out_max)
{
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}
*/


int analogRead(uint8_t pin)
{
	uint8_t low, high;
	
	
	ADMUX = pin;
	
	if(pin == 0b100010)//temperature sensor
	{
		ADMUX |= 0x80; //enable 1,1V reference voltage
	}
	
	// without a delay, we seem to read from the wrong channel
	//_delay_loop_2(10000);

	// start the first conversion
	ADCSRA |= (1 << ADSC);

	// ADSC is cleared when the conversion finishes
	while (ADCSRA & (1 << ADSC))
	{
		asm volatile("nop");
	}

	// we have to read ADCL first; doing so locks both ADCL
	// and ADCH until ADCH is read.  reading ADCL second would
	// cause the results of each conversion to be discarded,
	// as ADCL and ADCH would be locked when it completed.
	low = ADCL;
	high = ADCH;
	
	//the first conversion should not be used...
	
	
	/*
	// start the second conversion
	sbi(ADCSRA, ADSC);

	// ADSC is cleared when the conversion finishes
	while (bit_is_set(ADCSRA, ADSC));

	// we have to read ADCL first; doing so locks both ADCL
	// and ADCH until ADCH is read.  reading ADCL second would
	// cause the results of each conversion to be discarded,
	// as ADCL and ADCH would be locked when it completed.
	low = ADCL;
	high = ADCH;
	*/
	
	
	
	if(pin == 0b100010)//temperature sensor
	{
		ADMUX &= (uint8_t)~(0x80); //use VCC as reference voltage
	}

	// combine the two bytes
	return (high << 8) | low;
}

// Right now, PWM output only works on the pins with
// hardware support.  These are defined in the appropriate
// pins_*.c file.  For the rest of the pins, we default
// to digital output.
void analogWrite(uint8_t pin, int val)
{
	// We need to make sure the PWM output is enabled for those pins
	// that support it, as we turn it off when digitally reading or
	// writing with them.  Also, make sure the pin is in output mode
	// for consistenty with Wiring, which doesn't require a pinMode
	// call for the analog output pins.
	//pinMode(pin, OUTPUT);
	
	
	/*
  if(val > 255)
	{
		val = _map(val, 0, 1023, 0, 100);
	}
	else
	{
  	val = _map(val, 0, 255, 0, 100);
  }
  */
  
	//luminet uses bynase for analog output. pwm is not used!
	bynase_init(200);
	bynase_registerPin(pin, BYNASE_OUTPUT);
	bynase_write(pin, val);
	
	
	/*
	if (digitalPinToTimer(pin) == TIMER1A) {
		// connect pwm to pin on timer 1, channel A
		sbi(TCCR1A, COM1A1);
		// set pwm duty
		OCR1A = val;
	} else if (digitalPinToTimer(pin) == TIMER1B) {
		// connect pwm to pin on timer 1, channel B
		sbi(TCCR1A, COM1B1);
		// set pwm duty
		OCR1B = val;
	} else if (digitalPinToTimer(pin) == TIMER0A) {
    if (val == 0) {
      digitalWrite(pin, LOW);
    } else {
      // connect pwm to pin on timer 0, channel A
      sbi(TCCR0A, COM0A1);
      // set pwm duty
      OCR0A = val;      
    }
	} else if (digitalPinToTimer(pin) == TIMER0B) {
    if (val == 0) {
      digitalWrite(pin, LOW);
    } else {
      // connect pwm to pin on timer 0, channel B
      sbi(TCCR0A, COM0B1);
      // set pwm duty
      OCR0B = val;
    }
	}
	
	
	else if (val < 128)
		digitalWrite(pin, LOW);
	else
		digitalWrite(pin, HIGH);
	*/
	
	
	


	
		
}
