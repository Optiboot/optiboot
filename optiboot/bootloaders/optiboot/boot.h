/* Modified to use out for SPM access
** Peter Knight, Optiboot project http://optiboot.googlecode.com
**
** Todo: Tidy up
**
** "_short" routines execute 1 cycle faster and use 1 less word of flash
** by using "out" instruction instead of "sts".
**
** Additional elpm variants that trust the value of RAMPZ
*/

/* Copyright (c) 2002, 2003, 2004, 2005, 2006, 2007  Eric B. Weddington
   All rights reserved.

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions are met:

   * Redistributions of source code must retain the above copyright
     notice, this list of conditions and the following disclaimer.
   * Redistributions in binary form must reproduce the above copyright
     notice, this list of conditions and the following disclaimer in
     the documentation and/or other materials provided with the
     distribution.
   * Neither the name of the copyright holders nor the names of
     contributors may be used to endorse or promote products derived
     from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
  POSSIBILITY OF SUCH DAMAGE. */

/* $Id: boot.h,v 1.27.2.3 2008/09/30 13:58:48 arcanum Exp $ */

#ifndef _OPTIBOOT_BOOT_H_
#define _OPTIBOOT_BOOT_H_    1

/** \file */
/** \defgroup avr_boot <boot.h>: Optiboot Bootloader Support Utilities
    \code
    #include <boot.h>
    \endcode

    The macros in this module provide a C language interface to the
    bootloader support functionality of certain AVR processors. These
    macros are designed to work with all sizes of flash memory.
    The macros in this module are new additions to avr/boot.h and will
    redefine some functionality.
*/

#include <avr/boot.h>

#define __boot_page_fill_short(address, data)   \
(__extension__({                                 \
    __asm__ __volatile__                         \
    (                                            \
        "movw  r0, %3\n\t"                       \
        "out %0, %1\n\t"                         \
        "spm\n\t"                                \
        "clr  r1\n\t"                            \
        :                                        \
        : "i" (_SFR_IO_ADDR(__SPM_REG)),        \
          "r" ((uint8_t)__BOOT_PAGE_FILL),       \
          "z" ((uint16_t)address),               \
          "r" ((uint16_t)data)                   \
        : "r0"                                   \
    );                                           \
}))

#define __boot_page_fill_extended_short(address, data) \
(__extension__({                                 \
    __asm__ __volatile__                         \
    (                                            \
        "movw  r0, %4\n\t"                       \
        "movw r30, %A3\n\t"                      \
        "out %1, %C3\n\t"                        \
        "out %0, %2\n\t"                         \
        "spm\n\t"                                \
        "clr  r1\n\t"                            \
        :                                        \
        : "i" (_SFR_IO_ADDR(__SPM_REG)),        \
          "i" (_SFR_IO_ADDR(RAMPZ)),            \
          "r" ((uint8_t)__BOOT_PAGE_FILL),       \
          "r" ((uint32_t)address),               \
          "r" ((uint16_t)data)                   \
        : "r0", "r30", "r31"                     \
    );                                           \
}))

#define __boot_page_erase_short(address)        \
(__extension__({                                 \
    __asm__ __volatile__                         \
    (                                            \
        "out %0, %1\n\t"                         \
        "spm\n\t"                                \
        :                                        \
        : "i" (_SFR_IO_ADDR(__SPM_REG)),        \
          "r" ((uint8_t)__BOOT_PAGE_ERASE),      \
          "z" ((uint16_t)address)                \
    );                                           \
}))

#define __boot_page_erase_extended_short(address)      \
(__extension__({                                 \
    __asm__ __volatile__                         \
    (                                            \
        "movw r30, %A3\n\t"                      \
        "out  %1, %C3\n\t"                       \
        "out %0, %2\n\t"                         \
        "spm\n\t"                                \
        :                                        \
        : "i" (_SFR_IO_ADDR(__SPM_REG)),        \
          "i" (_SFR_IO_ADDR(RAMPZ)),            \
          "r" ((uint8_t)__BOOT_PAGE_ERASE),      \
          "r" ((uint32_t)address)                \
        : "r30", "r31"                           \
    );                                           \
}))

#define __boot_page_write_short(address)        \
(__extension__({                                 \
    __asm__ __volatile__                         \
    (                                            \
        "out %0, %1\n\t"                         \
        "spm\n\t"                                \
        :                                        \
        : "i" (_SFR_IO_ADDR(__SPM_REG)),        \
          "r" ((uint8_t)__BOOT_PAGE_WRITE),      \
          "z" ((uint16_t)address)                \
    );                                           \
}))

#define __boot_page_write_extended_short(address)      \
(__extension__({                                 \
    __asm__ __volatile__                         \
    (                                            \
        "movw r30, %A3\n\t"                      \
        "out %1, %C3\n\t"                        \
        "out %0, %2\n\t"                         \
        "spm\n\t"                                \
        :                                        \
        : "i" (_SFR_IO_ADDR(__SPM_REG)),        \
          "i" (_SFR_IO_ADDR(RAMPZ)),            \
          "r" ((uint8_t)__BOOT_PAGE_WRITE),      \
          "r" ((uint32_t)address)                \
        : "r30", "r31"                           \
    );                                           \
}))

#define __boot_rww_enable_short()                      \
(__extension__({                                 \
    __asm__ __volatile__                         \
    (                                            \
        "out %0, %1\n\t"                         \
        "spm\n\t"                                \
        :                                        \
        : "i" (_SFR_IO_ADDR(__SPM_REG)),        \
          "r" ((uint8_t)__BOOT_RWW_ENABLE)       \
    );                                           \
}))

#define __boot_lock_bits_set_short(lock_bits)                    \
(__extension__({                                           \
    uint8_t value = (uint8_t)(~(lock_bits));               \
    __asm__ __volatile__                                   \
    (                                                      \
        "ldi r30, 1\n\t"                                   \
        "ldi r31, 0\n\t"                                   \
        "mov r0, %2\n\t"                                   \
        "out %0, %1\n\t"                                   \
        "spm\n\t"                                          \
        :                                                  \
        : "i" (_SFR_IO_ADDR(__SPM_REG)),                  \
          "r" ((uint8_t)__BOOT_LOCK_BITS_SET),             \
          "r" (value)                                      \
        : "r0", "r30", "r31"                               \
    );                                                     \
}))

#define boot_lock_fuse_bits_get_short(address)                   \
(__extension__({                                           \
    uint8_t __result;                                      \
    __asm__ __volatile__                                   \
    (                                                      \
        "ldi r30, %3\n\t"                                  \
        "ldi r31, 0\n\t"                                   \
        "out %1, %2\n\t"                                   \
        "lpm %0, Z\n\t"                                    \
        : "=r" (__result)                                  \
        : "i" (_SFR_IO_ADDR(__SPM_REG)),                  \
          "r" ((uint8_t)__BOOT_LOCK_BITS_SET),             \
          "M" (address)                                    \
        : "r0", "r30", "r31"                               \
    );                                                     \
    __result;                                              \
}))

/*
The version of this macro has a minor difference vs the current
avr/boot.h, "uint8_t __result;", this should have no impact 
but optiboots original version retained to be safe.
boot_lock_fuse_bits_get is unused in optiboot.c so should have
no impact regardless.
*/
#ifndef IGNORE_OPTIBOOT_BOOT_MODIFICATIONS
#undef boot_lock_fuse_bits_get
#define boot_lock_fuse_bits_get(address)                   \
(__extension__({                                           \
    uint8_t __result;                                      \
    __asm__ __volatile__                                   \
    (                                                      \
        "ldi r30, %3\n\t"                                  \
        "ldi r31, 0\n\t"                                   \
        "sts %1, %2\n\t"                                   \
        "lpm %0, Z\n\t"                                    \
        : "=r" (__result)                                  \
        : "i" (_SFR_MEM_ADDR(__SPM_REG)),                  \
          "r" ((uint8_t)__BOOT_LOCK_BITS_SET),             \
          "M" (address)                                    \
        : "r0", "r30", "r31"                               \
    );                                                     \
    __result;                                              \
}))
#endif //IGNORE_OPTIBOOT_BOOT_MODIFICATIONS

#define boot_signature_byte_get_short(addr) \
(__extension__({                      \
      uint16_t __addr16 = (uint16_t)(addr);     \
      uint8_t __result;                         \
      __asm__ __volatile__                      \
      (                                         \
        "out %1, %2\n\t"                        \
        "lpm %0, Z" "\n\t"                      \
        : "=r" (__result)                       \
        : "i" (_SFR_IO_ADDR(__SPM_REG)),        \
          "r" ((uint8_t) __BOOT_SIGROW_READ),   \
          "z" (__addr16)                        \
      );                                        \
      __result;                                 \
}))

/*
The version of this macro has a minor difference vs the current
avr/boot.h, "uint16_t __addr16 = (uint16_t)(addr);", this should 
have no impact but optiboots original version retained to be safe.
boot_signature_byte_get is unused in optiboot.c so should have
no impact regardless.
*/
#ifndef IGNORE_OPTIBOOT_BOOT_MODIFICATIONS
#undef boot_signature_byte_get
#define boot_signature_byte_get(addr) \
(__extension__({                      \
      uint16_t __addr16 = (uint16_t)(addr);     \
      uint8_t __result;                         \
      __asm__ __volatile__                      \
      (                                         \
        "sts %1, %2\n\t"                        \
        "lpm %0, Z" "\n\t"                      \
        : "=r" (__result)                       \
        : "i" (_SFR_MEM_ADDR(__SPM_REG)),       \
          "r" ((uint8_t) __BOOT_SIGROW_READ),   \
          "z" (__addr16)                        \
      );                                        \
      __result;                                 \
}))
#endif //IGNORE_OPTIBOOT_BOOT_MODIFICATIONS


/* Normal versions of the macros use 16-bit addresses.
   Extended versions of the macros use 32-bit addresses.
   Alternate versions of the macros use 16-bit addresses and require special
   instruction sequences after LPM.
   "_short" routines execute 1 cycle faster and use 1 less word of flash
   by using "out" instruction instead of "sts". SPM_REG must be within 
   addressing range of OUT instruction to utilize.
   
   FLASHEND is defined in the ioXXXX.h file.
   USHRT_MAX is defined in <limits.h>. */ 

#if defined(__AVR_ATmega161__) || defined(__AVR_ATmega163__) \
    || defined(__AVR_ATmega323__) || defined(DISABLE_OPTIBOOT_BOOTPAGE_OVERRIDES)

/* Alternate: ATmega161/163/323 and 16 bit address */
#define optiboot_page_fill(address, data) boot_page_fill(address, data)
#define optiboot_page_erase(address)      boot_page_erase(address)
#define optiboot_page_write(address)      boot_page_write(address)
#define optiboot_rww_enable()             boot_rww_enable()
#define optiboot_lock_bits_set(lock_bits) boot_lock_bits_set(lock_bits)

#elif (FLASHEND > USHRT_MAX)

/* Extended: >16 bit address */
#define optiboot_page_fill(address, data) __boot_page_fill_extended_short(address, data)
#define optiboot_page_erase(address)      __boot_page_erase_extended_short(address)
#define optiboot_page_write(address)      __boot_page_write_extended_short(address)
#define optiboot_rww_enable()             __boot_rww_enable_short()
#define optiboot_lock_bits_set(lock_bits) __boot_lock_bits_set_short(lock_bits)

#else

/* Normal: 16 bit address */
#define optiboot_page_fill(address, data) __boot_page_fill_short(address, data)
#define optiboot_page_erase(address)      __boot_page_erase_short(address)
#define optiboot_page_write(address)      __boot_page_write_short(address)
#define optiboot_rww_enable()             __boot_rww_enable_short()
#define optiboot_lock_bits_set(lock_bits) __boot_lock_bits_set_short(lock_bits)

#endif




#endif /* _OPTIBOOT_BOOT_H_ */
