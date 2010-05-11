#ifndef _TOPOLOGY_H
#define _TOPOLOGY_H

#ifdef __cplusplus
extern "C"{
#endif


#include "luminet.h"


//performs a topology scan and vector detection 
//returns: connections variable: SNEWSNEW - first nibble is vector path, second nibble is connection indicator
//example: 01000001 means: a vector path is in the north and so I have two connections: one in the West and one (the vector path connection) in the North
void topology_scan(uint8_t*);

//If a vector is present, it sends a message to the vector path. This message is a single byte
//the byte is stored in the eeprom at address 228 and returned by this function
//uint8_t topology_getVectorMessage(void);

#ifdef __cplusplus
} // extern "C"
#endif

#endif

