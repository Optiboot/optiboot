#include "bynase.h"
#include <avr/io.h>

//number of samples simple_byin should take
#define NUMBER_OF_SAMPLES 200

#define NUMBER_OF_BYNASE_PINS 19

uint8_t outPinsA;//Arduino Pins 7..0
uint8_t outPinsB;//Arduino Pins 15..8
uint8_t outPinsC;//Arduino Pins 19..16

uint8_t inPinsA;//Arduino Pins 7..0
uint8_t inPinsB;//Arduino Pins 15..8
uint8_t inPinsC;//Arduino Pins 19..16

uint8_t byval_out[NUMBER_OF_BYNASE_PINS];//byvals for up to 19 output pins
uint8_t byval_in[NUMBER_OF_BYNASE_PINS];//byvals for up to 19 input pins

uint16_t total_in[NUMBER_OF_BYNASE_PINS];
uint8_t count_in[NUMBER_OF_BYNASE_PINS];

uint8_t timerSpeed;

uint8_t byrandValue;//for random


uint8_t byrand(void)
{
 uint8_t v = byrandValue++;
 uint8_t r = 0;
 uint8_t q = 255;
 while(q) {
 r = (r << 1) | (v & 1);
 v >>= 1;
 q >>= 1;
 }
 return r;
}

void _byin(uint8_t pinNumber)
{
	if(pinNumber < 8)
	{
		if(! (inPinsA & (1 <<(pinNumber))) )
		{
			return;//pin is not registered for bynase output
		}
	}
	else if(pinNumber < 15)
	{
		if(! (inPinsB & (1 <<(pinNumber-8))) )
		{
			return;//pin is not registered for bynase output
		}
	}
	else
	{
		if(! (inPinsC & (1 <<(pinNumber-15))) )
		{
			return;//pin is not registered for bynase output
		}
	}
	
	count_in[pinNumber]++;
	if(count_in[pinNumber] >= NUMBER_OF_SAMPLES)
	{
		byval_in[pinNumber] = (total_in[pinNumber] * 100) / count_in[pinNumber];
		count_in[pinNumber] = 0;
		total_in[pinNumber] = 0;
	}
	else
	{
		if(digitalRead(pinNumber) == HIGH)
		{
			total_in[pinNumber]++;
		}
		
	}	
}

void byin(void) 
{
	uint8_t z;
	for(z=0; z<NUMBER_OF_BYNASE_PINS; z++)
	{
		_byin(z);
	}
}

void _byout(uint8_t pinNumber, uint8_t random_value) 
{
	if(pinNumber < 8)
	{
		if(! (outPinsA & (1 <<pinNumber)) )
		{
			return;//pin is not registered for bynase input
		}
	}
	else if(pinNumber < 15)
	{
		if(! (outPinsB & (1 <<(pinNumber-8))) )
		{
			return;//pin is not registered for bynase input
		}
	}
	else
	{
		if(! (outPinsC & (1 <<(pinNumber-15))) )
		{
			return;//pin is not registered for bynase input
		}
	}
	
	
	if (byval_out[pinNumber] >= random_value) 
	{
		digitalWrite(pinNumber, HIGH);
	} 
	else 
	{
		digitalWrite(pinNumber, LOW);
	}
}

void byout(void) 
{
	uint8_t random_value;//only one random value for all ports
	random_value = ((byrand() % 99)+1);//random value between 1 and 100
	uint8_t z;
	for(z=0; z<NUMBER_OF_BYNASE_PINS; z++)
	{
		_byout(z, random_value);
	}
	
}

void bynase_init(uint8_t speed)
{
	uint8_t z;
	uint8_t oldSREG = SREG;
	
	if(timerSpeed != speed)
	{
		for(z=0; z<NUMBER_OF_BYNASE_PINS; z++)
		{
			byval_out[z] = 0;
			byval_in[z] = 0;
			total_in[z] = 0;
			count_in[z] = 0;
		}
		timerSpeed = speed;
		
		cli();	
		//INIT TIMER1
		TCCR1A = 0x00;
	  TCCR1B = 0b00000001; //no prescaler
	  TIMSK1 = 0b00000001;//enable timer overflow interrupt
	  
	 // byrandValue=0;
		
		// reenable interrupts.
		SREG = oldSREG;
	}
}

void bynase_write(uint8_t pinNumber, uint8_t value)
{
	byval_out[pinNumber] = value;
}


uint8_t bynase_read(uint8_t pinNumber)
{
	
	return byval_in[pinNumber];
}



void bynase_registerPin(uint8_t pinNumber, uint8_t mode)
{
	if(pinNumber > (NUMBER_OF_BYNASE_PINS-1))
	{
		return;
	}
	
	if(mode == BYNASE_INPUT)
	{
		pinMode(pinNumber, INPUT);
		digitalWrite(pinNumber, LOW);		

		if(pinNumber < 8)
		{
			inPinsA |= (1<<pinNumber);
			outPinsA &= (uint8_t)~(1<<pinNumber);
		}
		else if(pinNumber < 15)
		{
			inPinsB |= (1<<(pinNumber-8));
			outPinsB &= (uint8_t)~(1<<(pinNumber-8));
		}
		else
		{
			inPinsC |= (1<<(pinNumber-15));
			outPinsC &= (uint8_t)~(1<<(pinNumber-15));
		}
		
	}
	else if(mode == BYNASE_OUTPUT)
	{
		pinMode(pinNumber, OUTPUT);
		digitalWrite(pinNumber, LOW);
		
		if(pinNumber < 8)
		{
			outPinsA |= (1<<pinNumber);
			inPinsA &= (uint8_t)~(1<<pinNumber);
		}
		else if(pinNumber < 15)
		{
			outPinsB |= (1<<(pinNumber-8));
			inPinsB &= (uint8_t)~(1<<(pinNumber-8));
		}
		else
		{
			outPinsC |= (1<<(pinNumber-15));
			inPinsC &= (uint8_t)~(1<<(pinNumber-15));
		}
		
		
	}
	else if(mode == BYNASE_BOTH)
	{
		if(pinNumber < 8)
		{
			outPinsA |= (1<<pinNumber);
			inPinsA |= (1<<pinNumber);
		}
		else if(pinNumber < 15)
		{
			outPinsB |= (1<<(pinNumber-8));
			inPinsB |= (1<<(pinNumber-8));
		}
		else
		{
			outPinsC |= (1<<(pinNumber-15));
			inPinsC |= (1<<(pinNumber-15));
		}
	}
	
	else if(mode == BYNASE_NONE)
	{
		if(pinNumber < 8)
		{
			inPinsA &= (uint8_t)~(1<<pinNumber);
			outPinsA &= (uint8_t)~(1<<pinNumber);
		}
		else if(pinNumber < 15)
		{
			inPinsB &= (uint8_t)~(1<<(pinNumber-8));
			outPinsB &= (uint8_t)~(1<<(pinNumber-8));
		}
		else
		{
			inPinsC &= (uint8_t)~(1<<(pinNumber-15));
			outPinsC &= (uint8_t)~(1<<(pinNumber-15));
		}
	}
}

void bynase_unregisterPin(uint8_t pinNumber)
{
	bynase_registerPin(pinNumber, BYNASE_NONE);
}

#if defined (__AVR_ATtiny84__)
ISR(TIM1_OVF_vect)
#elif defined(__AVR_ATmega8__)
ISR(TIMER1_OVF_vect)
#elif defined (__AVR_ATmega168__)
ISR(TIMER1_OVF_vect)
#elif defined (__AVR_ATmega328P__)
ISR(TIMER1_OVF_vect)
#endif
{
	uint8_t oldSREG = SREG;
	cli();
	if( (inPinsA | inPinsB | inPinsC) >0)
	{
		byin();
	}
	
	if( (outPinsA |outPinsB | outPinsC) >0)
	{
		byout();
	}
	
	TCNT1 = 0xFFFF - (255-timerSpeed);//count 255-timerspeed times from now on at 1Mhz 
	TIFR1 = 0xFF;//clear all pending interrups for timer1
	
	// reenable interrupts.
	SREG = oldSREG;
}