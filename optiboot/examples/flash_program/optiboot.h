
/* Optiboot 'header' file.
 * 
 * June 2015 by Marek Wodzinski, https://github.com/majekw
 * Released to public domain
 * 
 * This header file gives possibility to use SPM instruction
 * from Optiboot bootloader memory.
 *
 * There are 3 convenient functions available here:
 *  optiboot_page_erase - to erase FLASH page
 *  optiboot_page_fill - to put words into temporary buffer
 *  optiboot_page_write - to write contents of temporary buffer into FLASH
 *
 * For some hardcore users, you could use 'do_spm' as raw entry to
 * bootloader spm function.
 */

#ifndef _OPTIBOOT_H_
#define _OPTIBOOT_H_  1

#include <avr/boot.h>

/* 
 * Main 'magic' function - enter to bootloader do_spm function
 *
 * address - address to write (in bytes) but must be even number
 * command - one of __BOOT_PAGE_WRITE, __BOOT_PAGE_ERASE or __BOOT_PAGE_FILL
 * data - data to write in __BOOT_PAGE_FILL. In __BOOT_PAGE_ERASE or 
 *          __BOOT_PAGE_WRITE it control if boot_rww_enable is run
 *         (0 = run, !0 = skip running boot_rww_enable)
 *
 *
 * Contents of bootloader's do_spm function, just for reference:
 *
 * static void do_spm(uint16_t address, uint8_t command, uint16_t data) {
 *     // Do spm stuff
 *     asm volatile (
 * 	 "    movw  r0, %3\n"
 *       "    out %0, %1\n"
 *       "    spm\n"
 *       "    clr  r1\n"
 *       :
 *       : "i" (_SFR_IO_ADDR(__SPM_REG)),
 *         "r" ((uint8_t)command),
 *         "z" ((uint16_t)address),
 *         "r" ((uint16_t)data)
 *       : "r0"
 *     );
 * 
 *     // wait for spm to complete
 *     //   it doesn't have much sense for __BOOT_PAGE_FILL,
 *     //   but it doesn't hurt and saves some bytes on 'if'
 *     boot_spm_busy_wait();
 * #if defined(RWWSRE)
 *     // this 'if' condition should be: (command == __BOOT_PAGE_WRITE || command == __BOOT_PAGE_ERASE)...
 *     // but it's tweaked a little assuming that in every command we are interested in here, there
 *     // must be also SELFPRGEN set. If we skip checking this bit, we save here 4B
 *     if ((command & (_BV(PGWRT)|_BV(PGERS))) && (data == 0) ) {
 *       // Reenable read access to flash
 *       boot_rww_enable();
 *     }
 * #endif
 * }
 *
 */
typedef void (*do_spm_t)(uint16_t address, uint8_t command, uint16_t data);
const do_spm_t do_spm = (do_spm_t)((FLASHEND-511+2)>>1);


/*
 * The same as do_spm but with disable/restore interrupts state
 * required to succesfull SPM execution
 */
void do_spm_cli(uint16_t address, uint8_t command, uint16_t data) {
  uint8_t sreg_save;

  sreg_save = SREG;  // save old SREG value
  asm volatile("cli");  // disable interrupts
  do_spm(address,command,data);
  SREG=sreg_save; // restore last interrupts state
}


/*
 * Erase page in FLASH
 */
void optiboot_page_erase(uint16_t address) {
  do_spm_cli(address,__BOOT_PAGE_ERASE,0);
}


/*
 * Write word into temporary buffer
 */
void optiboot_page_fill(uint16_t address, uint16_t data) {
  do_spm_cli(address,__BOOT_PAGE_FILL,data);
}


/*
 * Write temporary buffer into FLASH
 */
void optiboot_page_write(uint16_t address) {
  do_spm_cli(address,__BOOT_PAGE_WRITE,0);
}

#endif /* _OPTIBOOT_H_ */

