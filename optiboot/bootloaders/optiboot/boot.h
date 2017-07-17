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

/* $Id: optiboot_boot.h,v 1.27.2.3 2008/09/30 13:58:48 arcanum Exp $ */

#ifndef _BOOT_OPTIBOOT_H_
#define _BOOT_OPTIBOOT_H_    1

#include <avr/boot.h>

#define __boot_page_fill_short(address, data)   \
(__extension__({                                \
	__asm__ __volatile__                        \
	(                                           \
	"movw  r0, %3\n\t"							\
	"out %0, %1\n\t"							\
	"spm\n\t"									\
	"clr  r1\n\t"								\
	:											\
	: "i" (_SFR_IO_ADDR(__SPM_REG)),			\
	"r" ((uint8_t)__BOOT_PAGE_FILL),			\
	"z" ((uint16_t)address),					\
	"r" ((uint16_t)data)						\
	: "r0"										\
	);                                          \
}))


#define __boot_page_fill_extended_short(address, data) \
(__extension__({                                \
	__asm__ __volatile__                        \
	(                                           \
	"movw  r0, %4\n\t"							\
	"movw r30, %A3\n\t"							\
	"out %1, %C3\n\t"							\
	"out %0, %2\n\t"							\
	"spm\n\t"									\
	"clr  r1\n\t"								\
	:											\
	: "i" (_SFR_IO_ADDR(__SPM_REG)),			\
	"i" (_SFR_IO_ADDR(RAMPZ)),					\
	"r" ((uint8_t)__BOOT_PAGE_FILL),			\
	"r" ((uint32_t)address),					\
	"r" ((uint16_t)data)						\
	: "r0", "r30", "r31"						\
	);                                          \
}))


#define __boot_page_erase_short(address)        \
(__extension__({                                \
	__asm__ __volatile__                        \
	(                                           \
	"out %0, %1\n\t"							\
	"spm\n\t"									\
	:											\
	: "i" (_SFR_IO_ADDR(__SPM_REG)),			\
	"r" ((uint8_t)__BOOT_PAGE_ERASE),			\
	"z" ((uint16_t)address)						\
	);                                          \
}))


#define __boot_page_erase_extended_short(address) \
(__extension__({                                \
	__asm__ __volatile__                        \
	(                                           \
	"movw r30, %A3\n\t"							\
	"out  %1, %C3\n\t"							\
	"out %0, %2\n\t"							\
	"spm\n\t"									\
	:											\
	: "i" (_SFR_IO_ADDR(__SPM_REG)),			\
	"i" (_SFR_IO_ADDR(RAMPZ)),					\
	"r" ((uint8_t)__BOOT_PAGE_ERASE),			\
	"r" ((uint32_t)address)						\
	: "r30", "r31"								\
	);                                          \
}))

#define __boot_page_write_short(address)        \
(__extension__({                                \
	__asm__ __volatile__                        \
	(                                           \
	"out %0, %1\n\t"							\
	"spm\n\t"									\
	:											\
	: "i" (_SFR_IO_ADDR(__SPM_REG)),			\
	"r" ((uint8_t)__BOOT_PAGE_WRITE),			\
	"z" ((uint16_t)address)						\
	);                                          \
}))


#define __boot_page_write_extended_short(address) \
(__extension__({                                \
	__asm__ __volatile__                        \
	(                                           \
	"movw r30, %A3\n\t"							\
	"out %1, %C3\n\t"							\
	"out %0, %2\n\t"							\
	"spm\n\t"									\
	:											\
	: "i" (_SFR_IO_ADDR(__SPM_REG)),			\
	"i" (_SFR_IO_ADDR(RAMPZ)),					\
	"r" ((uint8_t)__BOOT_PAGE_WRITE),			\
	"r" ((uint32_t)address)						\
	: "r30", "r31"								\
	);                                          \
}))

#define __boot_rww_enable_short()               \
(__extension__({                                \
	__asm__ __volatile__                        \
	(                                           \
	"out %0, %1\n\t"							\
	"spm\n\t"									\
	:											\
	: "i" (_SFR_IO_ADDR(__SPM_REG)),			\
	"r" ((uint8_t)__BOOT_RWW_ENABLE)			\
	);                                          \
}))


#define __boot_lock_bits_set_short(lock_bits)   \
(__extension__({                                \
	uint8_t value = (uint8_t)(~(lock_bits));    \
	__asm__ __volatile__                        \
	(                                           \
	"ldi r30, 1\n\t"                            \
	"ldi r31, 0\n\t"                            \
	"mov r0, %2\n\t"                            \
	"out %0, %1\n\t"                            \
	"spm\n\t"                                   \
	:                                           \
	: "i" (_SFR_IO_ADDR(__SPM_REG)),            \
	"r" ((uint8_t)__BOOT_LOCK_BITS_SET),        \
	"r" (value)                                 \
	: "r0", "r30", "r31"                        \
	);                                          \
}))



#ifndef DISABLE_OPTIBOOT_BOOTPAGE_OVERRIDES

	#if defined(__AVR_ATmega161__) || defined(__AVR_ATmega163__) \
	|| defined(__AVR_ATmega323__)
		//unchanged
	#elif (FLASHEND > USHRT_MAX)
		//Undefine boot.h defines	
		#undef boot_page_fill
		#undef boot_page_erase
		#undef boot_page_write
		#undef boot_rww_enable
		#undef boot_lock_bits_set

		/* Extended: >16 bit address */		
		#define boot_page_fill(address, data) __boot_page_fill_extended_short(address, data)
		#define boot_page_erase(address)      __boot_page_erase_extended_short(address)
		#define boot_page_write(address)      __boot_page_write_extended_short(address)
		#define boot_rww_enable()             __boot_rww_enable_short()
		#define boot_lock_bits_set(lock_bits) __boot_lock_bits_set_short(lock_bits)
	#else	
		//Undefine boot.h defines
		#undef boot_page_fill
		#undef boot_page_erase
		#undef boot_page_write
		#undef boot_rww_enable
		#undef boot_lock_bits_set

		/* Normal: 16 bit address */		
		#define boot_page_fill(address, data) __boot_page_fill_short(address, data)
		#define boot_page_erase(address)      __boot_page_erase_short(address)
		#define boot_page_write(address)      __boot_page_write_short(address)
		#define boot_rww_enable()             __boot_rww_enable_short()
		#define boot_lock_bits_set(lock_bits) __boot_lock_bits_set_short(lock_bits)
	#endif

#endif //DISABLE_OPTIBOOT_BOOTPAGE_OVERRIDES







#endif /* _BOOT_OPTIBOOT_H_ */

