#ifndef _ASSEMBLERTEST_H_
#define _ASSEMBLERTEST_H_

#include "WProgram.h"

#ifdef __cplusplus
extern "C"{
#endif

extern void digitalWriteASM(uint8_t pin, uint8_t value);

void fastDigitalWrite(uint8_t pin, uint8_t value);


#ifdef __cplusplus
} // extern "C"
#endif


#endif /* _ASSEMBLERTEST_H_ */
