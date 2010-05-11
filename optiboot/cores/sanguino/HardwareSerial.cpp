/*
  HarwareSerial.cpp - Hardware serial library for Wiring
  Copyright (c) 2006 Nicholas Zambetti.  All right reserved.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
  
  Modified 23 November 2006 by David A. Mellis
  Modified 29 January 2009, Marius Kintel for Sanguino - http://www.sanguino.cc/
*/

#include "WProgram.h"

#include "wiring.h"
#include "HardwareSerial.h"

// Constructors ////////////////////////////////////////////////////////////////

HardwareSerial::HardwareSerial(uint8_t uart)
  :_uart(uart)
{
}

// Public Methods //////////////////////////////////////////////////////////////

void HardwareSerial::begin(long speed)
{
  beginSerial(_uart, speed);
}

uint8_t HardwareSerial::available(void)
{
  return serialAvailable(_uart);
}

int HardwareSerial::read(void)
{
  return serialRead(_uart);
}

void HardwareSerial::flush()
{
  serialFlush(_uart);
}

void HardwareSerial::write(uint8_t b) {
  serialWrite(_uart, b);
}

// Preinstantiate Objects //////////////////////////////////////////////////////

HardwareSerial Serial = HardwareSerial(0);

#if defined(__AVR_ATmega644P__)
HardwareSerial Serial1 = HardwareSerial(1);
#endif
