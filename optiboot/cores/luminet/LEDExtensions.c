#include "LEDExtensions.h"


void setRGB(uint8_t r, uint8_t g, uint8_t b)
{
	bynase_registerPin(8, BYNASE_OUTPUT);
	bynase_registerPin(9, BYNASE_OUTPUT);
	bynase_registerPin(10, BYNASE_OUTPUT);

	bynase_write(8, r);
	bynase_write(9, g);
	bynase_write(10, b);	
	
}