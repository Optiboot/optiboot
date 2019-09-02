/**********************************************************/
/* Optiboot bootloader for Arduino                        */
/*                                                        */
/* http://optiboot.googlecode.com                         */
/*                                                        */
/* Arduino-maintained version : See README.TXT            */
/* http://code.google.com/p/arduino/                      */
/*  It is the intent that changes not relevant to the     */
/*  Arduino production envionment get moved from the      */
/*  optiboot project to the arduino project in "lumps."   */
/*                                                        */
/* Heavily optimised bootloader that is faster and        */
/* smaller than the Arduino standard bootloader           */
/*                                                        */
/* Enhancements:                                          */
/*   Fits in 512 bytes, saving 1.5K of code space         */
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
/* Copyright 2013-2019 by Bill Westfield.                 */
/* Copyright 2010 by Peter Knight.                        */
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


/**********************************************************/
/*                                                        */
/* Optional defines:                                      */
/*                                                        */
/**********************************************************/
/*                                                        */
/* BIGBOOT:                                              */
/* Build a 1k bootloader, not 512 bytes. This turns on    */
/* extra functionality.                                   */
/*                                                        */
/* BAUD_RATE:                                             */
/* Set bootloader baud rate.                              */
/*                                                        */
/* LED_START_FLASHES:                                     */
/* Number of LED flashes on bootup.                       */
/*                                                        */
/* LED_DATA_FLASH:                                        */
/* Flash LED when transferring data. For boards without   */
/* TX or RX LEDs, or for people who like blinky lights.   */
/*                                                        */
/* TIMEOUT_MS:                                            */
/* Bootloader timeout period, in milliseconds.            */
/* 500,1000,2000,4000,8000 supported.                     */
/*                                                        */
/* UART:                                                  */
/* UART number (0..n) for devices with more than          */
/* one hardware uart (644P, 1284P, etc)                   */
/*                                                        */
/**********************************************************/

/**********************************************************/
/* Version Numbers!                                       */
/*                                                        */
/* Arduino Optiboot now includes this Version number in   */
/* the source and object code.                            */
/*                                                        */
/* Version 3 was released as zip from the optiboot        */
/*  repository and was distributed with Arduino 0022.     */
/* Version 4 starts with the arduino repository commit    */
/*  that brought the arduino repository up-to-date with   */
/*  the optiboot source tree changes since v3.            */
/*    :                                                   */
/* Version 9 splits off the Mega0/Xtiny support.          */
/*  This is very different from normal AVR because of     */
/*  changed peripherals and unified address space.        */
/*                                                        */
/* It would be good if versions implemented outside the   */
/*  official repository used an out-of-seqeunce version   */
/*  number (like 104.6 if based on based on 4.5) to       */
/*  prevent collisions.  The CUSTOM_VERSION=n option      */
/*  adds n to the high version to facilitate this.        */
/*                                                        */
/**********************************************************/

/**********************************************************/
/* Edit History:					  */
/*							  */
/* Aug 2019						  */
/* 9.0 Refactored for Mega0/Xtiny from optiboot.c         */
/*   :                                                    */
/* 4.1 WestfW: put version number in binary.		  */
/**********************************************************/

#define OPTIBOOT_MAJVER 9
#define OPTIBOOT_MINVER 0

/*
 * OPTIBOOT_CUSTOMVER should be defined (by the makefile) for custom edits
 * of optiboot.  That way you don't wind up with very different code that
 * matches the version number of a "released" optiboot.
 */

#if !defined(OPTIBOOT_CUSTOMVER)
#define OPTIBOOT_CUSTOMVER 0
#endif

unsigned const int __attribute__((section(".version"))) 
optiboot_version = 256*(OPTIBOOT_MAJVER + OPTIBOOT_CUSTOMVER) + OPTIBOOT_MINVER;


#include <inttypes.h>
#include <avr/io.h>

/*
 * optiboot uses several "address" variables that are sometimes byte pointers,
 * sometimes word pointers. sometimes 16bit quantities, and sometimes built
 * up from 8bit input characters.  avr-gcc is not great at optimizing the
 * assembly of larger words from bytes, but we can use the usual union to
 * do this manually.  Expanding it a little, we can also get rid of casts.
 */
typedef union {
    uint8_t  *bptr;
    uint16_t *wptr;
    uint16_t word;
    uint8_t bytes[2];
} addr16_t;


/*
 * pin_defs.h
 * This contains most of the rather ugly defines that implement our
 * ability to use UART=n and LED=D3, and some avr family bit name differences.
 */
#include "pin_defs_x.h"

/*
 * stk500.h contains the constant definitions for the stk500v1 comm protocol
 */
#include "stk500.h"

#ifndef LED_START_FLASHES
#define LED_START_FLASHES 0
#endif

/* set the UART baud rate defaults */
#ifndef BAUD_RATE
#define BAUD_RATE   115200L // Highest rate Avrdude win32 will support
#endif

#define BAUD_SETTING ((F_CPU*4) / (BAUD_RATE))
#define BAUD_ACTUAL ((64L*F_CPU)/(16L*BAUD_SETTING))

#if BAUD_ACTUAL <= BAUD_RATE
#define BAUD_ERROR (( 100*(BAUD_RATE - BAUD_ACTUAL) ) / BAUD_RATE)
#if BAUD_ERROR >= 5
#error BAUD_RATE off by greater than -5%
#elif BAUD_ERROR >= 2  && !defined(PRODUCTION)
#warning BAUD_RATE off by greater than -2%
#endif
#else
#define BAUD_ERROR (( 100*(BAUD_ACTUAL - BAUD_RATE) ) / BAUD_RATE)
#if BAUD_ERROR >= 5
#error BAUD_RATE off by greater than 5%
#elif BAUD_ERROR >= 2  && !defined(PRODUCTION)
#warning BAUD_RATE off by greater than 2%
#endif
#endif

#if BAUD_SETTING > 65635
#error Unachievable baud rate (too slow) BAUD_RATE 
#endif // baud rate slow check
#if (BAUD_SETTING - 1) < 3
#if BAUD_ERROR != 0 // permit high bitrates (ie 1Mbps@16MHz) if error is zero
#error Unachievable baud rate (too fast) BAUD_RATE 
#endif
#endif // baud rate fast check

/*
 * We can never load flash with more than 1 page at a time, so we can save
 * some code space on parts with smaller pagesize by using a smaller int.
 */
#if MAPPED_PROGMEM_PAGE_SIZE > 255
typedef uint16_t pagelen_t ;
#define GETLENGTH(len) len = getch()<<8; len |= getch()
#else
typedef uint8_t pagelen_t;
#define GETLENGTH(len) (void) getch() /* skip high byte */; len = getch()
#endif


/* Function Prototypes
 * The main() function is in init9, which removes the interrupt vector table
 * we don't need. It is also 'OS_main', which means the compiler does not
 * generate any entry or exit code itself (but unlike 'naked', it doesn't
 * supress some compile-time options we want.)
 */

void pre_main(void) __attribute__ ((naked)) __attribute__ ((section (".init8")));
int main(void) __attribute__ ((OS_main)) __attribute__ ((section (".init9"))) __attribute__((used));

void __attribute__((noinline)) __attribute__((leaf)) putch(char);
uint8_t __attribute__((noinline)) __attribute__((leaf)) getch(void) ;
void __attribute__((noinline)) verifySpace();
void __attribute__((noinline)) watchdogConfig(uint8_t x);

static void getNch(uint8_t);
#if LED_START_FLASHES > 0
static inline void flash_led(uint8_t);
#endif
static inline void watchdogReset();

/*
 * RAMSTART should be self-explanatory.  It's bigger on parts with a
 * lot of peripheral registers.  Let 0x100 be the default
 * Note that RAMSTART (for optiboot) need not be exactly at the start of RAM.
 */
#if !defined(RAMSTART)  // newer versions of gcc avr-libc define RAMSTART
#error RAMSTART not defined.
#endif

/* everything that needs to run VERY early */
void pre_main(void) {
    // Allow convenient way of calling do_spm function - jump table,
    //   so entry to this function will always be here, indepedent of compilation,
    //   features etc
    __asm__ __volatile__ (
	"	rjmp	1f\n"
#ifndef APP_NOSPM
	"	rjmp	do_nvmctrl\n"
#else
	"   ret\n"   // if do_spm isn't include, return without doing anything
#endif
	"1:\n"
	);
}


/* main program starts here */
int main(void) {
    uint8_t ch;

    /*
     * Making these local and in registers prevents the need for initializing
     * them, and also saves space because code no longer stores to memory.
     * (initializing address keeps the compiler happy, but isn't really
     *  necessary, and uses 4 bytes of flash.)
     */
    register addr16_t address;
    register pagelen_t  length;

    // After the zero init loop, this is the first code to run.
    //
    // This code makes the following assumptions:
    //  No interrupts will execute
    //  SP points to RAMEND
    //  r1 contains zero
    //
    // If not, uncomment the following instructions:
    // cli();
    __asm__ __volatile__ ("clr __zero_reg__");

    /*
     * Protect as much Reset Cause as possible for application
     * and still skip bootloader if not necessary
     */
    ch = RSTCTRL.RSTFR;

    // Skip all logic and run bootloader if cause is cleared (application request)
    if (ch != 0) {
	/*
	 * To run the boot loader, External Reset Flag must be set.
	 * If not, we could make shortcut and jump directly to application code.
	 * Also WDRF set with EXTRF is a result of Optiboot timeout, so we
	 * shouldn't run bootloader in loop :-) That's why:
	 *  1. application is running if WDRF is cleared
	 *  2. we clear WDRF if it's set with EXTRF to avoid loops
	 * One problematic scenario: broken application code sets watchdog timer 
	 * without clearing MCUSR before and triggers it quickly. But it's
	 * recoverable by power-on with pushed reset button.
	 */
	if ((ch & (RSTCTRL_WDRF_bm | RSTCTRL_EXTRF_bm)) != RSTCTRL_EXTRF_bm) { 
	    if (ch & RSTCTRL_EXTRF_bm) {
		/*
		 * Clear WDRF because it was most probably set by wdr in bootloader.
		 * It's also needed to avoid loop by broken application which could
		 * prevent entering bootloader.
		 * '&' operation is skipped to spare few bytes as bits in MCUSR
		 * can only be cleared.
		 */
		RSTCTRL.RSTFR = RSTCTRL_WDRF_bm;
	    }
	    /* 
	     * save the reset flags in the designated register
	     * This can be saved in a main program by putting code in .init0 (which
	     * executes before normal c init code) to save R2 to a global variable.
	     */
	    __asm__ __volatile__ ("mov r2, %0\n" :: "r" (ch));

	    // switch off watchdog
	    watchdogConfig(WDT_PERIOD_OFF_gc);
	    __asm__ __volatile__ (
		"jmp 512\n"
		);
	}
    }

#if LED_START_FLASHES > 0
    // Set up RTC counting at about 1/8s (input is 32kHz)
    RTC.CTRLA= RTC_PRESCALER_DIV4096_gc | RTC_RTCEN_bm;
    RTC.DBGCTRL = 1; // enable during debug
#endif

    _PROTECTED_WRITE(CLKCTRL.MCLKCTRLB, 0);  // full speed clock

    MYUART_TXPORT.DIR |= MYUART_TXPIN; // set TX pin to output
    MYUART_TXPORT.OUT |= MYUART_TXPIN;  // and "1" as per datasheet
#if defined (MYUART_PMUX)
    PORTMUX.USARTROUTEA |= MYUART_PMUX;  // alternate pinout to use
#endif
    MYUART.BAUD = BAUD_SETTING;
    MYUART.DBGCTRL = 1;  // run during debug
    MYUART.CTRLC = (USART_CHSIZE_gm & USART_CHSIZE_8BIT_gc);  // Async, Parity Disabled, 1 StopBit
    MYUART.CTRLA = 0;  // Interrupts: all off
    MYUART.CTRLB = USART_RXEN_bm | USART_TXEN_bm;

    // Set up watchdog to trigger after 1s
//    watchdogConfig(WDT_PERIOD_1KCLK_gc);

#if (LED_START_FLASHES > 0) || defined(LED_DATA_FLASH) || defined(LED_START_ON)
    /* Set LED pin as output */
    LED_PORT.DIR |= LED;
#endif

#if LED_START_FLASHES > 0
    /* Flash onboard LED to signal entering of bootloader */
    flash_led(LED_START_FLASHES * 2);
#else
#if defined(LED_START_ON)
    /* Turn on LED to indicate starting bootloader (less code!) */
    LED_PORT.OUT |= LED;
#endif
#endif

    /* Forever loop: exits by causing WDT reset */
    for (;;) {
	/* get character from UART */
	ch = getch();

	if(ch == STK_GET_PARAMETER) {
	    unsigned char which = getch();
	    verifySpace();
	    /*
	     * Send optiboot version as "SW version"
	     * Note that the references to memory are optimized away.
	     */
	    if (which == STK_SW_MINOR) {
		putch(optiboot_version & 0xFF);
	    } else if (which == STK_SW_MAJOR) {
		putch(optiboot_version >> 8);
	    } else {
		/*
		 * GET PARAMETER returns a generic 0x03 reply for
		 * other parameters - enough to keep Avrdude happy
		 */
		putch(0x03);
	    }
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
	    address.bytes[0] = getch();
	    address.bytes[1] = getch();
#ifdef RAMPZ
	    // Handle chips with more than 64k flash address space. (None yet)
	    // Transfer top bit to LSB in RAMPZ
#endif
	    address.word *= 2; // Convert from word address to byte address
	    verifySpace();
	}
	else if(ch == STK_UNIVERSAL) {
#ifndef RAMPZ
	    // UNIVERSAL command is ignored
	    getNch(4);
	    putch(0x00);
#endif
	}
	/* Write memory, length is big endian and is in bytes */
	else if(ch == STK_PROG_PAGE) {
	    // PROGRAM PAGE - we support flash programming only, not EEPROM
	    uint8_t desttype;

	    GETLENGTH(length);
	    desttype = getch();

	    if (desttype == 'F') {
		address.word += MAPPED_PROGMEM_START;
	    } else {
		address.word += MAPPED_EEPROM_START;
	    }

	    do
		*address.bptr++ = getch();
	    while (--length);

	    // Read command terminator, start reply
	    verifySpace();
	    /*
	     * Actually Write the buffer to flash (and wait for it to finish.)
	     */
	    _PROTECTED_WRITE_SPM(NVMCTRL.CTRLA, NVMCTRL_CMD_PAGEERASEWRITE_gc);
	    while (NVMCTRL.STATUS & (NVMCTRL_FBUSY_bm|NVMCTRL_EEBUSY_bm))
		; // wait for flash and EEPROM not busy, just in case.
	}
	/* Read memory block mode, length is big endian.  */
	else if(ch == STK_READ_PAGE) {
	    uint8_t desttype;
	    GETLENGTH(length);

	    desttype = getch();

	    verifySpace();
	    if (desttype == 'F') {
		address.word += MAPPED_PROGMEM_START;
	    } else {
		address.word += MAPPED_EEPROM_START;
	    }
	    do {
		putch(*(address.bptr++));
	    } while (--length);
	}

	/* Get device signature bytes  */
	else if(ch == STK_READ_SIGN) {
	    // READ SIGN - return what Avrdude wants to hear
	    verifySpace();
	    putch(SIGNATURE_0);
	    putch(SIGNATURE_1);
	    putch(SIGNATURE_2);
	}
	else if (ch == STK_LEAVE_PROGMODE) { /* 'Q' */
	    // Adaboot no-wait mod
	    watchdogConfig(WDT_PERIOD_8CLK_gc);
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
    while (0 == (MYUART.STATUS & USART_DREIF_bm))
	;
    MYUART.TXDATAL = ch;
}

uint8_t getch(void) {
    uint8_t ch, flags;
    while (!(MYUART.STATUS & USART_RXCIF_bm))
	;
    flags = MYUART.RXDATAH;
    ch = MYUART.RXDATAL;
    if ((flags & USART_FERR_bm) == 0)
	watchdogReset();
#ifdef LED_DATA_FLASH
    LED_PORT.IN |= LED;
#endif

    return ch;
}

void getNch(uint8_t count) {
    do getch(); while (--count);
    verifySpace();
}

void verifySpace() {
    if (getch() != CRC_EOP) {
	watchdogConfig(WDT_PERIOD_8CLK_gc);    // shorten WD timeout
	while (1)			      // and busy-loop so that WD causes
	    ;				      //  a reset and app start.
    }
    putch(STK_INSYNC);
}

#if LED_START_FLASHES > 0
void flash_led(uint8_t count) {
    uint8_t last;
    while (count--) {
	LED_PORT.IN |= LED;
	last = RTC.CNTL & 1;
	while ((RTC.CNTL & 1) == last) {
	    watchdogReset();
	    if (MYUART.STATUS & USART_RXCIF_bm)
		return;
	}
    }
}
#endif

// Watchdog functions. These are only safe with interrupts turned off.
void watchdogReset() {
    __asm__ __volatile__ (
	"wdr\n"
	);
}

void watchdogConfig(uint8_t x) {
    _PROTECTED_WRITE(WDT.CTRLA, x);
}




#ifndef APP_NOSPM

/*
 * Separate function for doing spm stuff
 * It's needed for application to do SPM, as SPM instruction works only
 * from bootloader.
 *
 * How it works:
 * - do SPM
 * - wait for SPM to complete
 * - if chip have RWW/NRWW sections it does additionaly:
 *   - if command is WRITE or ERASE, AND data=0 then reenable RWW section
 *
 * In short:
 * If you play erase-fill-write, just set data to 0 in ERASE and WRITE
 * If you are brave, you have your code just below bootloader in NRWW section
 *   you could do fill-erase-write sequence with data!=0 in ERASE and
 *   data=0 in WRITE
 */
static void do_nvmctrl(uint16_t address, uint8_t command, uint16_t data)  __attribute__ ((used));
static void do_nvmctrl(uint16_t address, uint8_t command, uint16_t data) {
    // Do spm stuff
}
#endif



#ifdef BIGBOOT
/*
 * Optiboot is designed to fit in 512 bytes, with a minimum feature set.
 * Some chips have a minimum bootloader size of 1024 bytes, and sometimes
 * it is desirable to add extra features even though 512bytes is exceedded.
 * In that case, the BIGBOOT can be used.
 * Our extra features so far don't come close to filling 1k, so we can
 * add extra "frivolous" data to the image.   In particular, we can add
 * information about how Optiboot was built (which options were selected,
 * what version, all in human-readable form (and extractable from the
 * binary with avr-strings.)
 * 
 * This can always be removed or trimmed if more actual program space
 * is needed in the future.  Currently the data occupies about 160 bytes,
 */
#define xstr(s) str(s)
#define str(s) #s
#define OPTFLASHSECT __attribute__((section(".fini8")))
#define OPT2FLASH(o) OPTFLASHSECT const char f##o[] = #o "=" xstr(o)


#ifdef LED_START_FLASHES
OPT2FLASH(LED_START_FLASHES);
#endif
#ifdef LED_DATA_FLASH
OPT2FLASH(LED_DATA_FLASH);
#endif
#ifdef LED_START_ON
OPT2FLASH(LED_START_ON);
#endif
#ifdef LED_NAME
OPTFLASHSECT const char f_LED[] = "LED=" LED_NAME;
#endif

#ifdef SUPPORT_EEPROM
OPT2FLASH(SUPPORT_EEPROM);
#endif
#ifdef BAUD_RATE
OPT2FLASH(BAUD_RATE);
#endif
#ifdef UART
OPT2FLASH(UART);
#endif

OPTFLASHSECT const char f_date[] = "Built:" __DATE__ ":" __TIME__;
#ifdef BIGBOOT
OPT2FLASH(BIGBOOT);
#endif
OPT2FLASH(F_CPU);
OPTFLASHSECT const char f_device[] = "Device=" xstr(__AVR_DEVICE_NAME__);
#ifdef OPTIBOOT_CUSTOMVER
OPT2FLASH(OPTIBOOT_CUSTOMVER);
#endif
OPTFLASHSECT const char f_version[] = "Version=" xstr(OPTIBOOT_MAJVER) "." xstr(OPTIBOOT_MINVER);

#endif
