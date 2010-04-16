/**********************************************************/
/* Optiboot bootloader for Arduino                        */
/*                                                        */
/* Heavily optimised bootloader that is faster and        */
/* smaller than the Arduino standard bootloader           */
/*                                                        */
/* Enhancements:                                          */
/*   Fits in 512 bytes, saving 1.5K of code space         */
/*   Background page erasing speeds up programming        */
/*   Higher baud rate speeds up programming               */
/*   Written almost entirely in C                         */
/*   Customisable timeout with accurate timeconstant      */
/*                                                        */
/* What you lose:                                         */
/*   Implements a skeleton STK500 protocol which is       */
/*     missing several features including EEPROM          */
/*     programming and non-page-aligned writes            */
/*   High baud rate breaks compatibility with standard    */
/*     Arduino flash settings                             */
/*                                                        */
/* Currently supports:                                    */
/*   ATmega168 based devices (Diecimila etc)              */
/*   ATmega328P based devices (Duemilanove etc)           */
/*                                                        */
/* Does not support:                                      */
/*   ATmega1280 based devices (eg. Mega)                  */
/*                                                        */
/* Assumptions:                                           */
/*   The code makes several assumptions that reduce the   */
/*   code size. They are all true after a hardware reset, */
/*   but may not be true if the bootloader is called by   */
/*   other means or on other hardware.                    */
/*     No interrupts can occur                            */
/*     UART and Timer 1 are set to their reset state      */
/*     SP points to RAMEND                                */
/*                                                        */
/* Code builds on code, libraries and optimisations from: */
/*   stk500boot.c          by Jason P. Kyle               */
/*   Arduino bootloader    http://arduino.cc              */
/*   Spiff's 1K bootloader http://spiffie.org/know/arduino_1k_bootloader/bootloader.shtml */
/*   avr-libc project      http://nongnu.org/avr-libc     */
/*   Adaboot               http://www.ladyada.net/library/arduino/bootloader.html */
/*                                                        */
/* This program is free software; you can redistribute it */
/* and/or modify it under the terms of the GNU General    */
/* Public License as published by the Free Software       */
/* Foundation; either version 2 of the License, or        */
/* (at your option) any later version.                    */
/*                                                        */
/* This program is distributed in the hope that it will   */
/* be useful, but WITHOUT ANY WARRANTY; without even the  */
/* implied warranty of MERCHANTABILITY or FITNESS FOR A   */
/* PARTICULAR PURPOSE.  See the GNU General Public        */
/* License for more details.                              */
/*                                                        */
/* You should have received a copy of the GNU General     */
/* Public License along with this program; if not, write  */
/* to the Free Software Foundation, Inc.,                 */
/* 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA */
/*                                                        */
/* Licence can be viewed at                               */
/* http://www.fsf.org/licenses/gpl.txt                    */
/*                                                        */
/**********************************************************/


#include <inttypes.h>
#include <avr/io.h>
#include <avr/pgmspace.h>
#include <avr/boot.h>

#define NOWAIT

/* Build-time variables */
/* BAUD_RATE       Programming baud rate */
/* LED_NO_FLASHES  Number of LED flashes on boot */
/* FLASH_TIME_MS   Duration of each LED flash */
/* BOOT_TIMEOUT_MS Serial port wait time before exiting bootloader */

/* set the UART baud rate */
#ifndef BAUD_RATE
#define BAUD_RATE   19200
#endif

/* Onboard LED is connected to pin PB5 in Arduino NG, Diecimila, and Duemilanove */ 
#define LED_DDR  DDRB
#define LED_PORT PORTB
#define LED_PIN  PINB
#define LED      PINB5

/* STK500 constants list, from AVRDUDE */
#define STK_OK              0x10
#define STK_FAILED          0x11  // Not used
#define STK_UNKNOWN         0x12  // Not used
#define STK_NODEVICE        0x13  // Not used
#define STK_INSYNC          0x14  // ' '
#define STK_NOSYNC          0x15  // Not used
#define ADC_CHANNEL_ERROR   0x16  // Not used
#define ADC_MEASURE_OK      0x17  // Not used
#define PWM_CHANNEL_ERROR   0x18  // Not used
#define PWM_ADJUST_OK       0x19  // Not used
#define CRC_EOP             0x20  // 'SPACE'
#define STK_GET_SYNC        0x30  // '0'
#define STK_GET_SIGN_ON     0x31  // '1'
#define STK_SET_PARAMETER   0x40  // '@'
#define STK_GET_PARAMETER   0x41  // 'A'
#define STK_SET_DEVICE      0x42  // 'B'
#define STK_SET_DEVICE_EXT  0x45  // 'E'
#define STK_ENTER_PROGMODE  0x50  // 'P'
#define STK_LEAVE_PROGMODE  0x51  // 'Q'
#define STK_CHIP_ERASE      0x52  // 'R'
#define STK_CHECK_AUTOINC   0x53  // 'S'
#define STK_LOAD_ADDRESS    0x55  // 'U'
#define STK_UNIVERSAL       0x56  // 'V'
#define STK_PROG_FLASH      0x60  // '`'
#define STK_PROG_DATA       0x61  // 'a'
#define STK_PROG_FUSE       0x62  // 'b'
#define STK_PROG_LOCK       0x63  // 'c'
#define STK_PROG_PAGE       0x64  // 'd'
#define STK_PROG_FUSE_EXT   0x65  // 'e'
#define STK_READ_FLASH      0x70  // 'p'
#define STK_READ_DATA       0x71  // 'q'
#define STK_READ_FUSE       0x72  // 'r'
#define STK_READ_LOCK       0x73  // 's'
#define STK_READ_PAGE       0x74  // 't'
#define STK_READ_SIGN       0x75  // 'u'
#define STK_READ_OSCCAL     0x76  // 'v'
#define STK_READ_FUSE_EXT   0x77  // 'w'
#define STK_READ_OSCCAL_EXT 0x78  // 'x'

/* Function Prototypes */
/* The main function is in init9, which removes the interrupt vector table */
/* we don't need. It is also 'naked', which means the compiler does not    */
/* generate any entry or exit code itself. */
int main(void) __attribute__ ((naked)) __attribute__ ((section (".init9")));
void putch(char);
uint8_t getch(void);
static inline void getNch(uint8_t); /* "static inline" is a compiler hint to reduce code size */
void verifySpace();
static inline void flash_led(uint8_t);
void setTimer(uint16_t counts);
uint8_t getLen();
void appStart() __attribute__ ((naked));

/* C zero initialises all global variables. However, that requires */
/* These definitions are NOT zero initialised, but that doesn't matter */
/* This allows us to drop the zero init code, saving us memory */
#define buff    ((uint8_t*)(0x100))
#define address (*(uint16_t*)(0x200))
#define length  (*(uint8_t*)(0x202))

/* main program starts here */
int main(void) {
  // After the zero init loop, this is the first code to run.
  //
  // This code makes the following assumptions:
  //  No interrupts will execute
  //  SP points to RAMEND
  //  r1 contains zero
  //
  // If not, uncomment the following instructions:
  // cli();
  // SP=RAMEND;  // This is done by hardware reset
  // asm volatile ("clr __zero_reg__");

  uint8_t ch;

#if 0
  // The compiler normally generates sts instructions for setting
  // up I/O space registers. But that takes 6 bytes per register.
  // This code can do it in 2, with an overhead of 20 bytes.
  // This starts to pay off when initialising 6 or more registers
  __asm__ volatile (
    "ldi r30,lo8(init_table)\n"
    "ldi r31,hi8(init_table)\n"
    "ldi r29,0\n"
    "init_loop:lpm r28,Z+\n"
    "lpm r0,Z+\n"
    "tst r28\n"
    "breq init_exit+2\n"
    "st Y,r0\n"
    "rjmp init_loop\n"
    "init_table:\n"
    ".byte %[addr1]\n"
    ".byte %[val1]\n"
    ".byte %[addr2]\n"
    ".byte %[val2]\n"
    ".byte %[addr3]\n"
    ".byte %[val3]\n"
    ".byte %[addr4]\n"
    ".byte %[val4]\n"
    ".byte %[addr5]\n"
    ".byte %[val5]\n"
    "init_exit:.word 0\n"
    // Clobbers R0,R28,R29,R30,R31
    // But that is fine so we won't tell the compiler
    // otherwise it will generate useless extra code
    ::
    [addr1]"i"(_SFR_MEM_ADDR(TCCR1B)), [val1]"i"(_BV(CS12) | _BV(CS10)),
    [addr2]"i"(_SFR_MEM_ADDR(UCSR0A)), [val2]"i"(_BV(U2X0)),
    [addr3]"i"(_SFR_MEM_ADDR(UCSR0B)), [val3]"i"(_BV(RXEN0) | _BV(TXEN0)),
    [addr4]"i"(_SFR_MEM_ADDR(UCSR0C)), [val4]"i"(_BV(UCSZ00) | _BV(UCSZ01)),
    [addr5]"i"(_SFR_MEM_ADDR(UBRR0L)), [val5]"i"((F_CPU + BAUD_RATE * 4L) / (BAUD_RATE * 8L) - 1)
  );
#else
  // Set up Timer 1 for timeout counter
  TCCR1B = _BV(CS12) | _BV(CS10); // div 1024
  UCSR0A = _BV(U2X0); //Double speed mode USART0
  UCSR0B = _BV(RXEN0) | _BV(TXEN0);
  UCSR0C = _BV(UCSZ00) | _BV(UCSZ01);
  UBRR0L = (uint8_t)( (F_CPU + BAUD_RATE * 4L) / (BAUD_RATE * 8L) - 1 );
#endif

  // Adaboot no-wait mod
  ch = MCUSR;
  MCUSR = 0;
  WDTCSR = _BV(WDCE) | _BV(WDE);
  WDTCSR = 0;
  if (!(ch & _BV(EXTRF))) appStart();

  /* set LED pin as output */
  LED_DDR |= _BV(LED);

  /* flash onboard LED to signal entering of bootloader */
  flash_led(NUM_LED_FLASHES * 2);

  /* forever loop */
  for (;;) {
    /* get character from UART */
    ch = getch();

    if(ch == STK_GET_PARAMETER) {
      // GET PARAMETER returns a generic 0x03 reply - enough to keep Avrdude happy
      getNch(1);
      putch(0x03);
    }
    else if(ch == STK_SET_DEVICE) {
      // SET DEVICE is ignored
      getNch(20);
    }
    else if(ch == STK_SET_DEVICE_EXT) {
      // SET DEVICE EXT is ignored
      getNch(5);
    }
    else if(ch == STK_LOAD_ADDRESS) {
      // LOAD ADDRESS
      address = getch();
      address = (address & 0xff) | (getch() << 8);
      address += address; // Convert from word address to byte address
      verifySpace();
    }
    else if(ch == STK_UNIVERSAL) {
      // UNIVERSAL command is ignored
      getNch(4);
      putch(0x00);
    }
    /* Write memory, length is big endian and is in bytes  */
    else if(ch == STK_PROG_PAGE) {
      // PROGRAM PAGE - we support flash programming only, not EEPROM
      uint8_t *bufPtr;
      uint16_t addrPtr;
      
      if (getLen() != 'F') appStart(); // Abort is not flash programming
  
      // Immediately start page erase - this will 4.5ms
      boot_page_erase((uint16_t)(void*)address);

      // While that is going on, read in page contents
      bufPtr = buff;
      do *bufPtr++ = getch();
      while (--length);

      // If only a partial page is to be programmed, the erase might not be complete.
      // So check that here
      boot_spm_busy_wait();

      // Read command terminator, start reply
      verifySpace();
      
      // Copy buffer into programming buffer
      bufPtr = buff;
      addrPtr = (uint16_t)(void*)address;
      ch = SPM_PAGESIZE / 2;
      do {
        uint16_t a;
        a = *bufPtr++;
        a |= (*bufPtr++) << 8;
        boot_page_fill((uint16_t)(void*)addrPtr,a);
        addrPtr += 2;
      } while (--ch);
      
      // Write from programming buffer
      boot_page_write((uint16_t)(void*)address);
      boot_spm_busy_wait();

      // Reenable read access to flash
      boot_rww_enable();
    }
    /* Read memory block mode, length is big endian.  */
    else if(ch == STK_READ_PAGE) {
      // READ PAGE - we only read flash
      getLen();
      verifySpace();
      do putch(pgm_read_byte_near(address++));
      while (--length);
    }

    /* Get device signature bytes  */
    else if(ch == STK_READ_SIGN) {
      // READ SIGN - return what Avrdude wants to hear
      verifySpace();
      putch(SIGNATURE_0);
      putch(SIGNATURE_1);
      putch(SIGNATURE_2);
    }
    else if (ch == 'Q') {
      // Adaboot no-wait mod
      WDTCSR = _BV(WDE);
      verifySpace();
    }
    else {
      // This covers the response to commands like STK_ENTER_PROGMODE
      verifySpace();
    }
    putch(STK_OK);
  }
}

void putch(char ch) {
  while (!(UCSR0A & _BV(UDRE0)));
  UDR0 = ch;
}

void setTimer(uint16_t counts) {
  TCNT1 = counts;
  TIFR1 = _BV(TOV1);
}

uint8_t getch(void) {
  LED_PORT &= ~_BV(LED);
  setTimer(-(F_CPU/(1024*2))); // 500ms
  while(!(UCSR0A & _BV(RXC0))) if (TIFR1 & _BV(TOV1)) appStart();
  LED_PORT |= _BV(LED);
  return UDR0;
}

void getNch(uint8_t count) {
  do getch(); while (--count);
  verifySpace();
}

void verifySpace() {
  if (getch() != CRC_EOP) appStart();
  putch(STK_INSYNC);
}

void flash_led(uint8_t count) {
  do {
    setTimer(-(F_CPU/(1024*16))); // 62ms
    while(!(TIFR1 & _BV(TOV1)));
    LED_PIN |= _BV(LED);
  } while (--count);
}

uint8_t getLen() {
  getch();
  length = getch();
  return getch();
}

void appStart() {
  __asm__ __volatile__ (
    "clr r30\n"
    "clr r31\n"
    "ijmp\n"
  );
}
