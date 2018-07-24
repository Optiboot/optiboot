/*
 * This short C progam creates a .elf file with EEPROM contents that
 * can be extracted to form a .hex file for testing EEPROM upload/downloads
 * via the bootloader.
 *
 * Compile with:
 *  avr-gcc -mmcu=atmega328p eeprom_data.c -o eeprom_data.elf
 *  avr-objcopy -j .eeprom --change-section-lma .eeprom=0 -O ihex eeprom_data.elf eeprom_data.hex
 * Upload with:
 *  avrdude ... -U eeprom:w:eeprom_data.hex
 */
#include <avr/io.h>
#include <avr/eeprom.h>

#define zero16 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0
#define zero32 zero16, zero16

const EEMEM unsigned long longs[32] = {0x12345678, 0x9abcdef0, 0x11223344, 0, 0xdeadbeef, zero16, 0xdecade, 0x78563412};
const EEMEM unsigned int integers[64] = {0x1234, 0x5678, 0x9ABC, 0, 0, 0, 1, 2 , zero32, 0xAB90, 0x3412};
const EEMEM char characters[128] = "test 1 2 3 4 5 6 7 8 9 0" "                                    "
    "more testing abcdefghijklmnop";


int main() {}

    
