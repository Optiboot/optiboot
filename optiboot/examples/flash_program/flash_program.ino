/* flash_program
 *
 * June 2015 by Marek Wodzinski, https://github.com/majekw
 * Released to public domain
 *
 * This is example how to use optiboot.h together with Optiboot
 * bootloader to write to FLASH memory by application code.
 *
 * IMPORTANT THINGS:
 * - buffer must be page aligned (see declaration of flash_buffer)
 * - interrupts must be disabled during spm
 * - writing to EEPROM destroys temporary buffer
 * - you can write only once into one location of temporary buffer
 * - only safely and always working sequence is erase-fill-write
 * - if you want to do fill-erase-write, you must put code in NRWW
 *     and pass data!=0 for erase. It's not easy, but possible.
 *
 * WRITE SEQUENCE - option 1 (used in this example)
 * 1. Erase page by optiboot_page_erase
 * 2. Write contents of page into temporary buffer by optiboot_page_fill
 * 3. Write temporary buffer to FLASH by optiboot_page_write
 *
 * WRITE SEQUENCE - option 2 (works only for code in NRWW)
 * 1. Write contents of page into temporary buffer by optiboot_page_fill
 * 2. Erase page by optiboot_page_erase (set data to NOT zero)
 * 3. Write temporary buffer to FLASH by optiboot_page_write
 */

#include "optiboot.h"
#include <avr/pgmspace.h>

const char flash_buffer[SPM_PAGESIZE] __attribute__ (( aligned(SPM_PAGESIZE) )) PROGMEM= {
  "0123456789ABCDEFGHIJKLMNOPQRSTUVW"
};

void setup() {
  int i;
  uint8_t c;
  uint16_t w;
  uint8_t ram_buffer[SPM_PAGESIZE];

  // Init serial
  Serial.begin(9600);
  
  // Print current flash buffer content
  Serial.println("Current flash contents:");
  for (i=0;i<SPM_PAGESIZE;i++) {
    c = pgm_read_byte(&flash_buffer[i]);
    if (c!=0 && c!=255) {
      Serial.write(c);
    } else {
      Serial.print(".");
    }
  }
  
  // Print prompt to enter some new characters to write to flash
  Serial.println();
  Serial.print("Type ");
  Serial.print(SPM_PAGESIZE);
  Serial.println(" characters to store in flash:");

  // Get characters from serial and put into ram buffer
  i=0;
  while (i<SPM_PAGESIZE) {
    if (Serial.available()>0) {
      c = Serial.read(); // read character from serial
      Serial.write(c); // echo character back
      ram_buffer[i] = c;
      i++;
    }
  }
  Serial.println("\nAll chars received");
  delay(100); // wait for sending all text via serial
  
  // Erasing FLASH page
  Serial.println("Erasing buffer");
  delay(100); // wait for sending all text via serial
  optiboot_page_erase((optiboot_addr_t)(void*) &flash_buffer[0]);

  // Copy ram buffer to temporary flash buffer
  Serial.println("Writing to temporary flash buffer");
  delay(100); // wait for sending all text via serial
  for (i=0;i<SPM_PAGESIZE;i++) {
    if (i % 2 == 0) { // we must write WORDs
      w = ram_buffer[i];
    } else {
      w += (ram_buffer[i] << 8);
      optiboot_page_fill((optiboot_addr_t)(void*) &flash_buffer[i],w);
    }
  }
  
  // Writing temporary buffer to FLASH
  Serial.println("Writing buffer to flash");
  delay(100); // wait for sending all text via serial
  optiboot_page_write((optiboot_addr_t)(void*) &flash_buffer[0]);

  Serial.println("Write done, thank you!");
  Serial.println("Now you can reset or power cycle board and check for new contents!");
}

void loop() {
  // no code here
}

