/**********************************************************/
/* Optiboot bootloader for Mega0, Tiny0, Tiny1            */
/*                                                        */
/*   https://github.com/optiboot/optiboot                 */
/*                                                        */
/* Heavily optimised bootloader that is fast and small    */
/* (512 bytes, 115200bps                                  */
/*                                                        */
/*   Written almost entirely in C                         */
/*   Customisable timeout with accurate timeconstant      */
/*                                                        */
/*                                                        */
/* Copyright 2013-2021 by Bill Westfield.                 */
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
/* https://github.com/Optiboot/optiboot/blob/master/LICENSE */
/*                                                        */
/**********************************************************/


/**********************************************************/
/*                                                        */
/* Optional defines:                                      */
/*                                                        */
/**********************************************************/
/*                                                        */
/* BIGBOOT:                                               */
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
/* WDTIME:                                                */
/* Bootloader timeout period, in seconds.                 */
/* 1, 2, 4, 8 supported.                                  */
/*                                                        */
/* UARTTX:                                                */
/* UART TX pin (B0, etc) for devices with more than       */
/* one hardware uart, or alternative pins                 */
/*                                                        */
/**********************************************************/

/**********************************************************/
/* Version Numbers!                                       */
/*                                                        */
/* Optiboot now includes a Version number in  the source  */
/*  and object code, and returns this value via STK500    */
/*                                                        */
/* The iniital Mega0/Xtiny support is version 9.          */
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
/* Edit History:                                          */
/*                                                        */
/* Jun 2021                                               */
/* 9.2 add rs485 support and SpenceKonde's startup code   */
/* Sep 2020                                               */
/* 9.1 fix do_nvmctrl                                     */
/* Aug 2019                                               */
/* 9.0 Refactored for Mega0/Xtiny from optiboot.c         */
/*   :                                                    */
/* 4.1 WestfW: put version number in binary.              */
/**********************************************************/

#define OPTIBOOT_MAJVER 9
#define OPTIBOOT_MINVER 2

/*
 * OPTIBOOT_CUSTOMVER should be defined (by the makefile) for custom edits
 * of optiboot.  That way you don't wind up with very different code that
 * matches the version number of a "released" optiboot.
 */

#if !defined(OPTIBOOT_CUSTOMVER)
# define OPTIBOOT_CUSTOMVER 0
#endif

unsigned const int __attribute__((section(".version"))) __attribute__((used))
optiboot_version = 256*(OPTIBOOT_MAJVER + OPTIBOOT_CUSTOMVER) + OPTIBOOT_MINVER;


#include <inttypes.h>
#include <avr/io.h>

#if (!defined(__AVR_XMEGA__)) || (__AVR_ARCH__ != 103)
#error CPU not supported by this version of Optiboot.
#include <unsupported>  // include a non-existent file to stop compilation
#endif

/*
 * Fuses.
 * This is an example of what they'd be like, but some should not
 * necessarily be under control of the bootloader.  You'll need a
 * a programmer that processes the .fuses section to actually get
 * these programmed into the chip.
 * The fuses actually REQUIRED by Optiboot are:
 *  BOOTEND=2, SYSCFG0=(CRC off, RSTPIN as appropriate)
 * On some chips, the "reset" pin can be either RESET or GPIO.
 *  Other also have the UPDI option. If RESET is not enabled we won't be
 *  able to auto-reset.  But if the UPDI pin is set to cause RESET, we
 *  won't be able to reprogram the chip without HV UPDI (which is uncommon.)
 * The settings show will set chips (ie m4809) with RESET/GPIO to use RESET,
 *  and chips with RESET/GPIO/UPDI to leave it in UPDI mode - the bootloader
 *  can still be started by a power-on RESET.
 */
FUSES = {
  .WDTCFG = 0,  /* Watchdog Configuration */
  .BODCFG = FUSE_BODCFG_DEFAULT,  /* BOD Configuration */
  .OSCCFG = FREQSEL_20MHZ_gc, /* 20MHz */
#ifdef FUSE_TCD0CFG_DEFAULT
  .TCD0CFG = FUSE_TCD0CFG_DEFAULT,  /* TCD0 Configuration */
#endif
#ifdef RSTPIN
  .SYSCFG0 =  CRCSRC_NOCRC_gc | RSTPINCFG_RST_gc, /* RESET is enabled */
#else
# ifdef FUSE_RSTPINCFG_gm  // group mask will be defined for triple-func pins
  .SYSCFG0 =  CRCSRC_NOCRC_gc | RSTPINCFG_UPDI_gc, /* RESET is not yet */
# else
  .SYSCFG0 =  CRCSRC_NOCRC_gc, /* RESET is not yet */
# endif
#endif
  .SYSCFG1 = 0x06,  /* startup 32ms */
  .APPEND = 0,  /* Application Code Section End */
  .BOOTEND = 2 /* Boot Section End */
};


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
# define LED_START_FLASHES 0
#endif

/*
 * The mega-0, tiny-0, and tiny-1 chips all reset to running on the
 *  internal oscillator, with a prescaler of 6.  The internal oscillator
 *  is either 20MHz or 16MHz, depending on a fuse setting - we can read
 *  the fuse to figure our which.
 * The BRG divisor is also fractional, permitting (afaik) any reasonable
 *  bit rate between about 1000bps and 115200bps. (higher bitrates would
 *  require changing the default clock.)
 * This makes the BRG generation a bit different than for prior processors.
 */
/* set the UART baud rate defaults */
#ifndef BAUD_RATE
# define BAUD_RATE   115200L // Highest rate Avrdude win32 will support
#endif
#ifdef F_CPU
# warning F_CPU is ignored for this chip (run from internal osc.)
#endif
#ifdef SINGLESPEED
# warning SINGLESPEED ignored for this chip. (Fractional BRG)
#endif
#ifdef UART
# warning UART is ignored for this chip (use UARTTX=PortPin instead)
#endif

#define BAUD_SETTING_16 (((16000000/6)*64) / (16L*BAUD_RATE))
#define BAUD_ACTUAL_16 ((64L*(16000000/6)) / (16L*BAUD_SETTING))
#define BAUD_SETTING_20 (((20000000/6)*64) / (16L*BAUD_RATE))
#define BAUD_ACTUAL_20 ((64L*(20000000/6)) / (16L*BAUD_SETTING))

#if BAUD_SETTING_16 < 64   // divisor must be > 1.  Low bits are fraction.
# error Unachievable baud rate (too fast) BAUD_RATE
#endif

#if BAUD_SETTING > 65635
# error Unachievable baud rate (too slow) BAUD_RATE
#endif // baud rate slow check

  /*
   * Watchdog timeout translations from human readable to config vals
   */
#ifndef WDTTIME
# define WDTPERIOD WDT_PERIOD_1KCLK_gc  // 1 second
#elif WDTTIME == 1
# define WDTPERIOD WDT_PERIOD_1KCLK_gc  // 1 second
#elif WDTTIME == 2
# define WDTPERIOD WDT_PERIOD_2KCLK_gc  // 2 seconds
#elif WDTTIME == 4
# define WDTPERIOD WDT_PERIOD_4KCLK_gc  // 4 seconds
#elif WDTTIME == 8
# define WDTPERIOD WDT_PERIOD_8KCLK_gc  // 8 seconds
#else
#endif

/*
 * We can never load flash with more than 1 page at a time, so we can save
 * some code space on parts with smaller pagesize by using a smaller int.
 */
#if MAPPED_PROGMEM_PAGE_SIZE > 255
typedef uint16_t pagelen_t;
# define GETLENGTH(len) len = getch()<<8; len |= getch()
#else
typedef uint8_t pagelen_t;
# define GETLENGTH(len) (void) getch() /* skip high byte */; len = getch()
#endif


/* Function Prototypes
 * The main() function is in init9, which removes the interrupt vector table
 * we don't need. It is also 'OS_main', which means the compiler does not
 * generate any entry or exit code itself (but unlike 'naked', it doesn't
 * suppress some compile-time options we want.)
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

#define watchdogReset()  __asm__ __volatile__ ("  wdr\n")

/*
 * RAMSTART should be self-explanatory.  It's bigger on parts with a
 * lot of peripheral registers.
 * Note that RAMSTART (for optiboot) need not be exactly at the start of RAM.
 */
#if !defined(RAMSTART)  // newer versions of gcc avr-libc define RAMSTART
#error RAMSTART not defined.
#endif

/* everything that needs to run VERY early */
void pre_main (void) {
  // Allow convenient way of calling do_spm function - jump table,
  //   so entry to this function will always be here, independent
  //    of compilation, features, etc
  __asm__ __volatile__ (
    "  rjmp 1f\n"
#ifndef APP_NOSPM
    "  rjmp do_nvmctrl\n"
#else
    "  ret\n"   // if do_spm isn't include, return without doing anything
#endif
    "1:\n"
    );
}

/* main program starts here */
int main (void) {
  uint8_t ch;

  /*
   * Making these local and in registers prevents the need for initializing
   * them, and also saves space because code no longer stores to memory.
   * (initializing address keeps the compiler happy, but isn't really
   *  necessary, and uses 4 bytes of flash.)
   */
  register addr16_t address;
  register pagelen_t  length;

  // This is the first code to run.
  //
  // Optiboot C code makes the following assumptions:
  //  No interrupts will execute
  //  SP points to RAMEND

  __asm__ __volatile__ ("  clr __zero_reg__\n"); // known-zero required by avr-libc
#define RESET_EXTERNAL (RSTCTRL_EXTRF_bm|RSTCTRL_UPDIRF_bm|RSTCTRL_SWRF_bm)
#ifndef FANCY_RESET_LOGIC
  ch = RSTCTRL.RSTFR;   // get reset cause
#ifdef START_APP_ON_POR
  /*
   * If WDRF is set OR nothing except BORF and PORF are set, that's
   * not bootloader entry condition so jump to app - this is for when
   * UPDI pin is used as reset, so we go straight to app on start.
   * 11/14: NASTY bug - we also need to check for no reset flags being
   * set (ie, direct entry) and run bootloader in that case, otherwise
   * bootloader won't run, among other things, after fresh bootloading!
   */

  if (ch && (ch & RSTCTRL_WDRF_bm ||
             (!(ch & (~(RSTCTRL_BORF_bm | RSTCTRL_PORF_bm)))))) {
# ifdef KeepBracesMatched
  }
# endif
#else
  /*
   * If WDRF is set OR nothing except BORF is set, that's not
   * bootloader entry condition so jump to app - let's see if this
   * works okay or not...
   */
  if (ch && (ch & RSTCTRL_WDRF_bm || (!(ch & (~RSTCTRL_BORF_bm))))) {
#endif
    /* Start the app.
     * Dont bother trying to stuff it in r2, which requires heroic
     * effort to fish out we'll put it in GPIOR0 where it won't get
     * stomped on.
     */
    // __asm__ __volatile__ ("  mov r2, %0\n" :: "r" (ch));
    RSTCTRL.RSTFR = ch; //clear the reset causes before jumping to app...
    GPIOR0 = ch; // but, stash the reset cause in GPIOR0 for use by app...
    watchdogConfig(WDT_PERIOD_OFF_gc);
    __asm__ __volatile__(
      "  jmp app\n"
      );
  }
#else
  /*
   * Protect as much Reset Cause as possible for application
   * and still skip bootloader if not necessary
   */
  ch = RSTCTRL.RSTFR;
  if (ch != 0) {
    /*
     * We want to run the bootloader when an external reset has occurred.
     * On these mega0/XTiny chips, there are three types of ext reset:
     * reset pin (may not exist), UPDI reset, and SW-request reset.
     * One of these reset causes, together with watchdog reset, should
     * mean that Optiboot timed out, and it's time to run the app.
     * Other reset causes (notably poweron) should run the app directly.
     * If a user app wants to utilize and detect watchdog resets, it
     * must make sure that the other reset causes are cleared.
     */
    if (ch & RSTCTRL_WDRF_bm) {
      if (ch & RESET_EXTERNAL) {
        /*
         * Clear WDRF because it was most probably set by wdr in
         * bootloader.  It's also needed to avoid loop by broken
         * application which could prevent entering bootloader.
         */
        RSTCTRL.RSTFR = RSTCTRL_WDRF_bm;
      }
    }
    if (!(ch & RESET_EXTERNAL)) {
      /*
       * save the reset flags in the designated register.
       * This can be saved in a main program by putting code in
       * .init0 (which executes before normal c init code) to save R2
       * to a global variable.
       */
      __asm__ __volatile__("  mov r2, %0\n" :: "r"(ch));

      // switch off watchdog
      watchdogConfig(WDT_PERIOD_OFF_gc);
      __asm__ __volatile__(
        "  jmp app\n"
        );
    }
  }
#endif // Fancy reset cause stuff

  watchdogReset();
//    _PROTECTED_WRITE(CLKCTRL.MCLKCTRLB, 0);  // full speed clock

  MYUART_TXPORT.DIR |= MYUART_TXPIN; // set TX pin to output
  MYUART_TXPORT.OUT |= MYUART_TXPIN;  // and "1" as per datasheet
#if defined (MYUART_PMUX_VAL)
  MYPMUX_REG = MYUART_PMUX_VAL;  // alternate pinout to use
#endif
  if ((FUSE_OSCCFG & FUSE_FREQSEL_gm) == FREQSEL_16MHZ_gc) {
    MYUART.BAUD = BAUD_SETTING_16;
  } else {
    MYUART.BAUD = BAUD_SETTING_20;
  }
  MYUART.DBGCTRL = 1;  // run during debug
  MYUART.CTRLC = (USART_CHSIZE_gm & USART_CHSIZE_8BIT_gc);  // Async, Parity Disabled, 1 StopBit
  MYUART.CTRLA = 0;  // Interrupts: all off
  MYUART.CTRLB = USART_RXEN_bm | USART_TXEN_bm;

#ifdef RS485
  RS485_PORT.DIR |= _BV(RS485_BIT);
#endif

  // Set up watchdog to trigger after a bit
  //  (nominally:, 1s for autoreset, longer for manual)
  watchdogConfig(WDTPERIOD);

#if (LED_START_FLASHES > 0) || defined(LED_DATA_FLASH) || defined(LED_START_ON)
  /* Set LED pin as output */
  LED_PORT.DIR |= LED;
#endif

#if LED_START_FLASHES > 0
  /* Flash onboard LED to signal entering of bootloader */
# ifdef LED_INVERT
  flash_led(LED_START_FLASHES * 2+1);
# else
  flash_led(LED_START_FLASHES * 2);
# endif
#else
#if defined(LED_START_ON)
# ifndef LED_INVERT
  /* Turn on LED to indicate starting bootloader (less code!) */
  LED_PORT.OUT |= LED;
# endif
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
      // ToDo: will there be mega-0 chips with >128k of RAM?
/*          UPDI chips apparently have byte-addressable FLASH ?
            address.word *= 2; // Convert from word address to byte address
*/
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
      // PROGRAM PAGE - any kind of page!
      uint8_t desttype;

      GETLENGTH(length);
      desttype = getch();

      if (desttype == 'F') {
        address.word += MAPPED_PROGMEM_START;
      } else {
        address.word += MAPPED_EEPROM_START;
      }
      // TODO: user row?

      do {
        *(address.bptr++) = getch();
      } while (--length);

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
      // TODO: user row?

      do {
        putch(*(address.bptr++));
      } while (--length);
    }

    /* Get device signature bytes */
    else if(ch == STK_READ_SIGN) {
      // READ SIGN - return actual device signature from SIGROW
      // this enables the same binary to be ued on multiple chips.
      verifySpace();
      putch(SIGROW_DEVICEID0);
      putch(SIGROW_DEVICEID1);
      putch(SIGROW_DEVICEID2);
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

#if RS485
inline void rs485_txon() {
# ifdef RS485_INVERT
  RS485_PORT.OUT &= ~_BV(RS485_BIT);
# else
  RS485_PORT.OUT |= _BV(RS485_BIT);
# endif
}
inline void rs485_txoff() {
// First, wait for any pending transmits to finish.
  MYUART.STATUS = USART_TXCIF_bm;  // This clears the TX complete flag
  while ((MYUART.STATUS & USART_TXCIF_bm) == 0)
    ; // spin loop waiting for TX Complete (could be immediately)
# ifdef RS485_INVERT
  RS485_PORT.OUT |= _BV(RS485_BIT);
# else
  RS485_PORT.OUT &= ~_BV(RS485_BIT);
# endif
}
#else // provide null functions to be optimized away.
inline void rs485_txon() {}
inline void rs485_txoff() {}
#endif

void putch (char ch) {
  rs485_txon();  // turn on and leave on till we're done xmitting.
  while (0 == (MYUART.STATUS & USART_DREIF_bm))
    ;
  MYUART.TXDATAL = ch;
  rs485_txoff();    // To receive, turn off transmitter
}

uint8_t getch (void) {
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

void getNch (uint8_t count) {
  do getch(); while (--count);
  verifySpace();
}

void verifySpace () {
  if (getch() != CRC_EOP) {
    watchdogConfig(WDT_PERIOD_8CLK_gc);    // shorten WD timeout
    while (1)                         // and busy-loop so that WD causes
      ;                               //  a reset and app start.
  }
  putch(STK_INSYNC);
}

#if LED_START_FLASHES > 0
void flash_led (uint8_t count) {
  uint16_t delay;  // at 20MHz/6, a 16bit delay counter is enough
  while (count--) {
    LED_PORT.IN |= LED;
    // delay assuming 20Mhz OSC.  It's only to "look about right", anyway.
    for (delay = ((20E6/6)/150); delay; delay--) {
      watchdogReset();
      if (MYUART.STATUS & USART_RXCIF_bm)
        return;
    }
  }
  watchdogReset(); // for breakpointing
}
#endif


/*
 * Change the watchdog configuration.
 *  Could be a new timeout, could be off...
 */
void watchdogConfig (uint8_t x) {
  while(WDT.STATUS & WDT_SYNCBUSY_bm)
    ;  // Busy wait for sycnhronization is required!
  _PROTECTED_WRITE(WDT.CTRLA, x);
}


#ifndef APP_NOSPM
/*
 * Separate function for doing nvmctrl stuff.
 * It's needed for application to do manipulate flash, since only the
 *  bootloader can write or erase flash, or write to the flash alias areas.
 * Note that this is significantly different in the details than the
 *  do_spm() function provided on older AVRs.  Same "vector", though.
 *
 * How it works:
 * - if the "command" is legal, write it to NVMCTRL.CTRLA
 * - if the command is not legal, store data to *address
 * - wait for NVM to complete
 *
 * For example, to write a flash page:
 * Copy each byte with
 *   do_nvmctrl(flashOffset+MAPPED_PROGMEM_START, 0xFF, *inputPtr);
 * Erase and write page with
 *   do_nvmctrl(0, NVMCTRL_CMD_PAGEERASEWRITE_gc, 0);
 */
static void do_nvmctrl(uint16_t address, uint8_t command, uint8_t data)  __attribute__ ((used));
static void do_nvmctrl (uint16_t address, uint8_t command, uint8_t data) {
  if (command <= NVMCTRL_CMD_gm) {
    _PROTECTED_WRITE_SPM(NVMCTRL.CTRLA, command);
    while (NVMCTRL.STATUS & (NVMCTRL_FBUSY_bm|NVMCTRL_EEBUSY_bm))
      ; // wait for flash and EEPROM not busy, just in case.
  } else {
    *(uint8_t *)address = data;
  }
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

__attribute__((section(".fini9"))) const char f_delimit = 0xFF;

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

#if defined(RS485)
OPTFLASHSECT const char f_rs485[] = "RS485=" RS485_NAME;
#endif
#ifdef BAUD_RATE
OPT2FLASH(BAUD_RATE);
#endif
#ifdef UARTTX
OPTFLASHSECT const char f_uart[] = "UARTTX=" UART_NAME;
#endif

OPTFLASHSECT const char f_date[] = "Built:" __DATE__ ":" __TIME__;
#ifdef BIGBOOT
OPT2FLASH(BIGBOOT);
#endif
OPTFLASHSECT const char f_device[] = "Device=" xstr(__AVR_DEVICE_NAME__);
#ifdef OPTIBOOT_CUSTOMVER
# if OPTIBOOT_CUSTOMVER != 0
OPT2FLASH(OPTIBOOT_CUSTOMVER);
# endif
#endif
OPTFLASHSECT const char f_version[] = "Version=" xstr(OPTIBOOT_MAJVER) "." xstr(OPTIBOOT_MINVER);

#endif

// Dummy application that will loop back into the bootloader if not overwritten
// This gives the bootloader somewhere to jump, and by referencing otherwise
//  unused variables/functions in the bootloader, it prevents them from being
//  omitted by the linker, with fewer mysterious link options.
void  __attribute__((section( ".application")))
      __attribute__((naked)) app();
void app()
{
  uint8_t ch;

  ch = RSTCTRL.RSTFR;
  RSTCTRL.RSTFR = ch; // reset causes
  *(volatile uint16_t *)(&optiboot_version);   // reference the version
  do_nvmctrl(0, NVMCTRL_CMD_PAGEBUFCLR_gc, 0); // reference this function!
  __asm__ __volatile__ (
    "  jmp 0\n"     // similar to running off end of memory
    );
//    _PROTECTED_WRITE(RSTCTRL.SWRR, 1); // cause new reset
}
