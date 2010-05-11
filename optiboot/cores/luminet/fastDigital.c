#include "fastDigital.h"

void fastDigitalWrite(uint8_t pin, uint8_t value)
{
	digitalWriteASM(pin, value);
}