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
                          __attribute__ ((used))
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
  
  Serial.println("Reset flag test");
  
  Serial.print("Have reset flag value 0x");
  Serial.print(resetFlags, HEX);
  
  /*
   * check for the usual bits.  Note that the symnbols defined in wdt.h are
   * bit numbers, so they have to be shifted before comparison.
   */
  if (resetFlags & (1<<WDRF))
  {
    Serial.print(" Watchdog");
    resetFlags &= ~(1<<WDRF);
  }
  if (resetFlags & (1<<BORF))
  {
    Serial.print(" Brownout");
    resetFlags &= ~(1<<BORF);
  }
  if (resetFlags & (1<<EXTRF))
  {
    Serial.print(" External");
    resetFlags &= ~(1<<EXTRF);
  }
  if (resetFlags & (1<<PORF))
  {
    Serial.print(" PowerOn");
    resetFlags &= ~(1<<PORF);
  }
  if (resetFlags != 0x00)
  {
    // It should never enter here
    Serial.print(" Unknown");
  }
  Serial.println("");
}

void loop() {
  Serial.println("Send something to reset through watchdog peripheral");
  while (Serial.read() < 0) ;
  wdt_enable(WDTO_15MS);
  while (1); // To prevent the loop to start again before WDT resets the board
}

