/*------------------------ Optiboot header file ----------------------------|
 |                                                                          |
 | June 2015 by Marek Wodzinski, https://github.com/majekw                  |
 | Modified June 2016 by MCUdude, https://github.com/MCUdude                |
 | Released to public domain                                                |
 |                                                                          |
 | This header file gives possibility to use SPM instruction                |
 | from Optiboot bootloader memory.                                         |
 |                                                                          |
 | There are 6 convenient functions available here:                         |
 | * optiboot_page_erase - to erase a FLASH page                            |
 | * optiboot_page_fill - to put words into temporary buffer                |
 | * optiboot_page_write - to write contents of temporary buffer into FLASH |
 | * optiboot_readPage - higher level function to read a flash page and     |
 | 	                      store it in an array                              |
 | * optiboot_writePage - higher level function to write content to         |
 |                         a flash page                                     |
 | * optiboot_copy - copy FLASH memory from one location to another, and    |
 |                    then call to a new address.  Useful for flashing new  |
 |                    firmware on to your board.                            |
 |                                                                          |
 | For some hardcore users, you could use 'do_spm' as raw entry to          |
 | bootloader spm function.                                                 |
 |-------------------------------------------------------------------------*/


#ifndef _OPTIBOOT_H_
#define _OPTIBOOT_H_  1

#include <avr/boot.h>
#include "Arduino.h"


/* 
 * Main 'magic' function - enter to bootloader do_spm function
 *
 * address - address to write (in bytes) but must be even number
 * command - one of __BOOT_PAGE_WRITE, __BOOT_PAGE_ERASE or __BOOT_PAGE_FILL
 * data - data to write in __BOOT_PAGE_FILL. In __BOOT_PAGE_ERASE or 
 *          __BOOT_PAGE_WRITE it control if boot_rww_enable is run
 *         (0 = run, !0 = skip running boot_rww_enable)
 *
 */

// 'typedef' (in following line) and 'const' (few lines below) are a way to define external function at some arbitrary address
typedef void (*do_spm_t)(uint16_t address, uint8_t command, uint16_t data);
typedef void(*retFunc_t)(void);
typedef void(*do_spm_copy_t)(uint16_t dst, uint16_t src, uint8_t numPages, retFunc_t retFunc);


/*
 * Devices with more than 64KB of flash:
 * - have larger bootloader area (1KB) (they are BIGBOOT targets)
 * - have RAMPZ register :-) 
 * - need larger variable to hold address (pgmspace.h uses uint32_t)
 */
#ifdef RAMPZ
  typedef uint32_t optiboot_addr_t;
#else
  typedef uint16_t optiboot_addr_t;
#endif

#if (FLASHEND > 65534) || defined(BIGBOOT)
  const do_spm_t do_spm = (do_spm_t)((FLASHEND-1023+2)>>1);
  const do_spm_copy_t do_spm_copy = (do_spm_copy_t)((FLASHEND - 1023 + 4) >> 1);
#else
  const do_spm_t do_spm = (do_spm_t)((FLASHEND-511+2)>>1);
#endif


/*
 * The same as do_spm but with disable/restore interrupts state
 * required to succesfull SPM execution
 * 
 * On devices with more than 64kB flash, 16 bit address is not enough,
 * so there is also RAMPZ used in that case.
 */
void do_spm_cli(optiboot_addr_t address, uint8_t command, uint16_t data) {
  uint8_t sreg_save;

  sreg_save = SREG;  // save old SREG value
  asm volatile("cli");  // disable interrupts
  #ifdef RAMPZ
    RAMPZ = (address >> 16) & 0xff;  // address bits 23-16 goes to RAMPZ
    do_spm((address & 0xffff), command, data); // do_spm accepts only lower 16 bits of address
  #else
    do_spm(address, command, data);  // 16 bit address - no problems to pass directly
  #endif
  SREG = sreg_save; // restore last interrupts state
}


// Erase page in FLASH
void optiboot_page_erase(optiboot_addr_t address) {
  do_spm_cli(address, __BOOT_PAGE_ERASE, 0);
}


// Write word into temporary buffer
void optiboot_page_fill(optiboot_addr_t address, uint16_t data) {
  do_spm_cli(address, __BOOT_PAGE_FILL, data);
}


//Write temporary buffer into FLASH
void optiboot_page_write(optiboot_addr_t address) {
  do_spm_cli(address, __BOOT_PAGE_WRITE, 0);
}



/*
 * Higher level functions for reading and writing from flash 
 * See the examples for more info on how to use these functions
 */

// Function to read a flash page and store it in an array (storage_array[])
void optiboot_readPage(const uint8_t allocated_flash_space[], uint8_t storage_array[], uint16_t page, char blank_character)
{
  uint8_t read_character;
  for(uint16_t j = 0; j < SPM_PAGESIZE; j++) 
  {
    read_character = pgm_read_byte(&allocated_flash_space[j + SPM_PAGESIZE*(page-1)]);
    if(read_character != 0 && read_character != 255)
      storage_array[j] = read_character;
    else
      storage_array[j] = blank_character;
  }
}


// Function to read a flash page and store it in an array (storage_array[]), but without blank_character
void optiboot_readPage(const uint8_t allocated_flash_space[], uint8_t storage_array[], uint16_t page)
{
  uint8_t read_character;
  for(uint16_t j = 0; j < SPM_PAGESIZE; j++) 
  {
    read_character = pgm_read_byte(&allocated_flash_space[j + SPM_PAGESIZE*(page-1)]);
    if(read_character != 0 && read_character != 255)
      storage_array[j] = read_character;
  }
}


// Function to write data to a flash page
void optiboot_writePage(const uint8_t allocated_flash_space[], uint8_t data_to_store[], uint16_t page)
{
  uint16_t word_buffer = 0; 

  // Erase the flash page
  optiboot_page_erase((optiboot_addr_t)(void*) &allocated_flash_space[SPM_PAGESIZE*(page-1)]);

  // Copy ram buffer to temporary flash buffer
  for(uint16_t i = 0; i < SPM_PAGESIZE; i++)
  {
    if(i % 2 == 0) // We must write words
      word_buffer = data_to_store[i];
    else
    {
      word_buffer += (data_to_store[i] << 8);
      optiboot_page_fill((optiboot_addr_t)(void*) &allocated_flash_space[i + SPM_PAGESIZE*(page-1)], word_buffer);
    }
  }

  // Writing temporary buffer to flash
  optiboot_page_write((optiboot_addr_t)(void*) &allocated_flash_space[SPM_PAGESIZE*(page-1)]);
}


#ifdef BIGBOOT
// Copy from one part of FLASH to another
void optiboot_copy(optiboot_addr_t dst, optiboot_addr_t src, uint8_t numPages, retFunc_t retFunc) {
  uint8_t sreg_save;

  sreg_save = SREG;  // save old SREG value
  asm volatile("cli");  // disable interrupts
  do_spm_copy(dst, src, numPages, retFunc);
  SREG = sreg_save; // restore last interrupts state
}
#endif

#endif /* _OPTIBOOT_H_ */
