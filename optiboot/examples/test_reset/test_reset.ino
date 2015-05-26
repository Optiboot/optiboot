/*
 * test_reset
 * May 2015 by Bill Westfield (WestfW)
 * Released to the public domain.
 *
 * This sketch demonstrates retrival of the Reset Cause register (MCUSR) of the AVR.
 * Normally, MCUSR itself is destroyed by the use of a bootloader, but Optiboot v4.6
 * and later save the contents in register r2, where it can be accessed by an
 * application.
 */
#include <avr/wdt.h>

/*
 * First, we need a variable to hold the reset cause that can be written before
 * early sketch initialization (that might change r2), and won't be reset by the
 * various initialization code.
 * avr-gcc provides for this via the ".noinit" section.
 */
uint8_t resetFlags __attribute__ ((section(".noinit")));

/*
 * Next, we need to put some code to save reset cause from the bootload (in r2)
 * to the variable.  Again, avr-gcc provides special code sections for this.
 */
void resetFlagsInit(void) __attribute__ ((naked))
                          __attribute__ ((section (".init0")));
void resetFlagsInit(void)
{
  /*
   * save the reset flags passed from the bootloader
   * This is a "simple" matter of storing (STS) r2 in the special variable
   * that we have created.  We use assembler to access the right variable.
   */
  __asm__ __volatile__ ("sts %0, r2\n" : "=m" (resetFlags) :);
}

void setup() {
  Serial.begin(9600);  // Initialize serial port
}

void loop() {
  Serial.println("Reset flag test");
  while (Serial.read() < 0)
    ;  // wait for some type-in

  Serial.print("Have reset flag value 0x");
  Serial.print(resetFlags, HEX);
  /*
   * check for the usual bits.  Note that the symnbols defined in wdt.h are
   * bit numbers, so they have to be shifted before comparison.
   */
  switch (resetFlags) {
    case 1<<WDRF:
      Serial.println(" (Watchdog)\n");
      break;
    case 1<<BORF:
      Serial.println(" (Brownout)\n");
      break;
    case 1<<EXTRF:
      Serial.println(" (External Reset)\n");
      break;
    case  1<<PORF:
    case ((1<<PORF) | (1<<BORF)):
      // A poweron can frequently result in both PORF and BORF being set, as the
      //  power switch bounces or the voltage rises slowly.
      Serial.println(" (PowerOn)\n");
      break;
    default:
      // Multiple bits can be set under various circumstances.  For example, the
      //  "hold down reset while powering up" trick used to recover from certain
      //  problems can leave both PORF and EXTRF set.
      Serial.println(" (Unknown)\n");
      break;
  }
  /*
   * Now we'll let the Watchdog reset the chip, and see if that cause is reported
   * correctly.  Older versions of optiboot could not easily distinguish between
   * WDRF and EXTRF resets, since the WDT was used inside the bootloader.
   */
  wdt_enable(WDTO_1S);
}

