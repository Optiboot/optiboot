/*
 * defs.h
 *
 * Created: 5/4/2017 4:49:17 PM
 *  Author: charl
 */


/*
 * This minimal bootloader does not support many basic functions, such as reading fuses.
 * So the user should not be surprised if returned values are incorrect, such as zero
 * values for fuses. The bootloader supports bootloading, reading flash, reading eeprom,
 * and not much else.
*/

#ifndef DEFS_H_
#define DEFS_H_

#define F_CPU 8000000L
#define BAUD_RATE 9600
#define TARGET atmega328
#define  MCU_TARGET atmega328p
#define AVR_FREQ 8000000L

/*#define VIRTUAL_BOOT_PARTITION */

#endif /* DEFS_H_ */