/*
  wiring_serial.c - serial functions.
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

#include "COM.h"
#include "wiring_private.h"


// Define constants and variables for buffering incoming serial data.  We're
// using a ring buffer (I think), in which rx_buffer_head is the index of the
// location to which to write the next incoming character and rx_buffer_tail
// is the index of the location from which to read.
//#define RX_BUFFER_SIZE 128//NOT SUPPORTED

//unsigned char rx_buffer[RX_BUFFER_SIZE];//NOT SUPPORTED

//int rx_buffer_head = 0;//NOT SUPPORTED
//int rx_buffer_tail = 0;//NOT SUPPORTED

///////////////////////////////////////////////////////////////////
/// Luminet does not support hardware serial
///////////////////////////////////////////////////////////////////

	void (*ptr_putc) (uint8_t data);
	uint8_t (*ptr_getc) (void);


void beginSerial(long baud)
{
 //baud NOT SUPPORTED... always 9600baud at 1MHz
 ptr_putc = LEFT_putc;
 ptr_getc = LEFT_getc;
 COM_init();
}

void selectDirectionSerial(uint8_t direction)
{
	if(direction == WEST)
	{
		ptr_putc = LEFT_putc;
 		ptr_getc = LEFT_getc;
	}
	else if(direction == EAST)
	{
		ptr_putc = RIGHT_putc;
 		ptr_getc = RIGHT_getc;
	}
	else if(direction == NORTH)
	{
		ptr_putc = UP_putc;
 		ptr_getc = UP_getc;
	}
	else if(direction == SOUTH)
	{
		ptr_putc = DOWN_putc;
 		ptr_getc = DOWN_getc;
	}
	
}

void serialWrite(unsigned char c)
{
	//NOT SUPPORTED
	ptr_putc(c);
}

int serialAvailable()
{
	//NOT SUPPORTED
	return 1;
}

int serialRead()
{
	//NOT SUPPORTED
	return ptr_getc();
}

void serialFlush()
{
	// don't reverse this or there may be problems if the RX interrupt
	// occurs after reading the value of rx_buffer_head but before writing
	// the value to rx_buffer_tail; the previous value of rx_buffer_head
	// may be written to rx_buffer_tail, making it appear as if the buffer
	// were full, not empty.
	//rx_buffer_head = rx_buffer_tail;
}

/*
//NOT SUPPORTED

#if defined(__AVR_ATmega8__)
SIGNAL(SIG_UART_RECV)
#else
SIGNAL(USART_RX_vect)
#endif
{
#if defined(__AVR_ATmega8__)
	unsigned char c = UDR;
#else
	unsigned char c = UDR0;
#endif

	int i = (rx_buffer_head + 1) % RX_BUFFER_SIZE;

	// if we should be storing the received character into the location
	// just before the tail (meaning that the head would advance to the
	// current location of the tail), we're about to overflow the buffer
	// and so we don't write the character or advance the head.
	if (i != rx_buffer_tail) {
		rx_buffer[rx_buffer_head] = c;
		rx_buffer_head = i;
	}
}
*/