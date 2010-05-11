#ifndef WProgram_h
#define WProgram_h

#include <stdlib.h>
#include <string.h>
#include <math.h>

#include <avr/interrupt.h>

#include "topology.h"
#include "bynase.h"
#include "LEDExtensions.h"
#include "COM.h"

#include "wiring.h"

uint8_t mainconnections;


#ifdef __cplusplus
#include "HardwareSerial.h"

uint16_t makeWord(uint16_t w);
uint16_t makeWord(byte h, byte l);

#define word(...) makeWord(__VA_ARGS__)

unsigned long pulseIn(uint8_t pin, uint8_t state, unsigned long timeout = 1000000L);

// WMath prototypes
long random(long);
long random(long, long);
void randomSeed(unsigned int);
long map(long, long, long, long, long);
#endif

#endif