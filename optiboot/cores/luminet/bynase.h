#ifndef _BYNASE_H
#define _BYNASE_H


#include "WProgram.h"

#ifdef __cplusplus
extern "C"{
#endif



#define BYNASE_INPUT 0
#define BYNASE_OUTPUT 1
#define BYNASE_BOTH 2
#define BYNASE_NONE 3

void bynase_init(uint8_t speed);
void byout(void);
void byin(void);
void bynase_write(uint8_t, uint8_t);
uint8_t bynase_read(uint8_t);
void bynase_registerPin(uint8_t pinNumber, uint8_t mode);
void bynase_unregisterPin(uint8_t pinNumber);

#ifdef __cplusplus
} // extern "C"
#endif

#endif