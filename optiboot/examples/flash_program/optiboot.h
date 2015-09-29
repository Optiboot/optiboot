
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

// 'typedef' (in following line) and 'const' (few lines below) are a way to define external function at some arbitrary address
typedef void (*do_spm_t)(uint16_t address, uint8_t command, uint16_t data);

/*
 * Devices with more than 64KB of flash:
 * - have larger bootloader area (1KB) (they are BIGBOOT targets)
 * - have RAMPZ register :-) 
 * - need larger variable to hold address (pgmspace.h uses uint32_t)
 * 
 * To not do many ifdefs and don't confuse users I declared new 'always valid'
 * type to declare address: optiboot_addr_t.
 */
#ifdef RAMPZ
  typedef uint32_t optiboot_addr_t;
  const do_spm_t do_spm = (do_spm_t)((FLASHEND-1023+2)>>1);
#else
  typedef uint16_t optiboot_addr_t;
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
    RAMPZ=(address>>16) & 0xff;  // address bits 23-16 goes to RAMPZ
    do_spm((address & 0xffff),command,data); // do_spm accepts only lower 16 bits of address
  #else
    do_spm(address,command,data);  // 16 bit address - no problems to pass directly
  #endif
  SREG=sreg_save; // restore last interrupts state
}

/*
 * Erase page in FLASH
 */
void optiboot_page_erase(optiboot_addr_t address) {
  do_spm_cli(address,__BOOT_PAGE_ERASE,0);
}


/*
 * Write word into temporary buffer
 */
void optiboot_page_fill(optiboot_addr_t address, uint16_t data) {
  do_spm_cli(address,__BOOT_PAGE_FILL,data);
}


/*
 * Write temporary buffer into FLASH
 */
void optiboot_page_write(optiboot_addr_t address) {
  do_spm_cli(address,__BOOT_PAGE_WRITE,0);
}

#endif /* _OPTIBOOT_H_ */

