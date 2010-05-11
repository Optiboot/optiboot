#ifndef _COM_H_
#define _COM_H_

#include "WProgram.h"

#ifdef __cplusplus
extern "C"{
#endif


#include <util/delay_basic.h>



#define WEST 0
#define EAST 1
#define NORTH 2
#define SOUTH 3

#ifndef BOOTADR
#define BOOTADR 0x1A00
#endif


void COM_init(void);

//destination: WEST, EAST, NORTH, SOUTH
//c: char to be send
void COM_putc(uint8_t destination, uint8_t c);

//source: WEST, EAST, NORTH, SOUTH
uint8_t COM_getc(uint8_t source);

void LEFT_putc(uint8_t c);//DO NOT USE!
uint8_t LEFT_getc(void);//DO NOT USE!

void RIGHT_putc(uint8_t c);//DO NOT USE!
uint8_t RIGHT_getc(void);//DO NOT USE!

void UP_putc(uint8_t c);//DO NOT USE!
uint8_t UP_getc(void);//DO NOT USE!

void DOWN_putc(uint8_t c);//DO NOT USE!
uint8_t DOWN_getc(void);//DO NOT USE!


//DO NOT USE THESE FUNCTIONS:
//use them only in the bootloader code


extern void COM_initASM(void);

extern void LEFT_putcASM(uint8_t c);//DO NOT USE!
extern uint8_t LEFT_getcASM(void);//DO NOT USE!

extern void RIGHT_putcASM(uint8_t c);//DO NOT USE!
extern uint8_t RIGHT_getcASM(void);//DO NOT USE!

extern void UP_putcASM(uint8_t c);//DO NOT USE!
extern uint8_t UP_getcASM(void);//DO NOT USE!

extern void DOWN_putcASM(uint8_t c);//DO NOT USE!
extern uint8_t DOWN_getcASM(void);//DO NOT USE!

#ifdef __cplusplus
} // extern "C"
#endif


#endif /* _COM_H_ */
