/*
   test_reset
   May 2015 by Bill Westfield (WestfW)
   Released to the public domain.

   This sketch demonstrates retrival of the Reset Cause register (MCUSR) of the AVR.
   Normally, MCUSR itself is destroyed by the use of a bootloader, but Optiboot v4.6
   and later save the contents in register r2, where it can be accessed by an
   application.
*/
#include <avr/wdt.h>
#if defined (__AVR_ATmega328P__) || defined (__AVR_ATmega168__)
#include <LowPower.h>
#endif

/*
   First, we need a variable to hold the reset cause that can be written before
   early sketch initialization (that might change r2), and won't be reset by the
   various initialization code.
   avr-gcc provides for this via the ".noinit" section.
*/
uint8_t resetFlag __attribute__ ((section(".noinit")));

/*
   Next, we need to put some code to save reset cause from the bootload (in r2)
   to the variable.  Again, avr-gcc provides special code sections for this.
   If compiled with link time optimization (-flto), as done by the Arduno
   IDE version 1.6 and higher, we need the "used" attribute to prevent this
   from being omitted.
*/
void resetFlagsInit(void) __attribute__ ((naked))
__attribute__ ((used))
__attribute__ ((section (".init0")));
void resetFlagsInit(void)
{
  /*
     save the reset flags passed from the bootloader
     This is a "simple" matter of storing (STS) r2 in the special variable
     that we have created.  We use assembler to access the right variable.
  */
  __asm__ __volatile__ ("sts %0, r2\n" : "=m" (resetFlag) :);
}

void printReset(const char *label, uint8_t resetFlags)
{
  Serial.print(label);
  Serial.print(resetFlags, HEX);
  /*
     check for the usual bits.  Note that the symnbols defined in wdt.h are
     bit numbers, so they have to be shifted before comparison.
  */
  if (resetFlags & (1 << WDRF))
  {
    Serial.print(F(" Watchdog"));
    resetFlags &= ~(1 << WDRF);
  }
  if (resetFlags & (1 << BORF))
  {
    Serial.print(F(" Brownout"));
    resetFlags &= ~(1 << BORF);
  }
  if (resetFlags & (1 << EXTRF))
  {
    Serial.print(F(" External"));
    resetFlags &= ~(1 << EXTRF);
  }
  if (resetFlags & (1 << PORF))
  {
    Serial.print(F(" PowerOn"));
    resetFlags &= ~(1 << PORF);
  }
  if (resetFlags != 0x00)
  {
    // It should never enter here
    Serial.print(" Unknown");
  }
  Serial.println("");
}

void setup() {
  Serial.begin(9600);  // Initialize serial port

  Serial.println(F("Reset flag test\n"));

  printReset("Actual MCUSR content: 0x", MCUSR);
  printReset("Passed in R2: 0x", resetFlag);
#ifdef GPIOR0
  printReset("Passed in GPIOR0: 0x", GPIOR0);
#endif
}

void loop() {
  int ch;
  Serial.println(F("\nSend 0-9 to set MCUSR, (W)atchdog enable, (J)mp to bootload, (S)leep"));
  while ((ch = Serial.read()) < 0) ;
  if (ch >= '0' && ch <= '9') {
    MCUSR = ch & 0xF;
    printReset("\nNew MCUSR content: 0x", MCUSR);
  } else
    switch (ch & ~('a' - 'A')) {
      
      case 'W':
        wdt_enable(WDTO_15MS);
        while (1); // To prevent the loop to start again before WDT resets the board
        break;
      case 'S':
#if defined (__AVR_ATmega328P__) || defined (__AVR_ATmega168__)
        // Enter power down state for 8 s with ADC and BOD module disabled
        Serial.println(F("Low Power Sleep Mode"));
        Serial.flush();
        LowPower.powerDown(SLEEP_8S, ADC_OFF, BOD_OFF);
        Serial.println(F("Sleep Wakeup"));
        Serial.flush();
#else
        Serial.print(F("\nPower down not supported on this CPU\n"));
#endif
        break;
        
      case 'J':
        /* Figure out where the bootloader starts. */
#if FLASHEND > 140000
        Serial.println(F("Jump to bootloader not supported on chips with >128k memory"));
#else
        typedef void (*do_reboot_t)(void);
        const do_reboot_t do_reboot = (do_reboot_t)((FLASHEND - 511) >> 1);

        Serial.print("bootstart = ");
        Serial.println((unsigned int)do_reboot, HEX);
        Serial.flush();
        cli(); TCCR0A = TCCR1A = TCCR2A = 0; // make sure interrupts are off and timers are reset.
        do_reboot();
#endif
        break;
    }

}

