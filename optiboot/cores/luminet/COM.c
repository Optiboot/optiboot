#include "COM.h"

//void (*startBootloader)( void ) = (void *)BOOTADR;  // pointer to the bootloader start address

void COM_init(void)
{
	uint8_t sreg = SREG;
	cli();
	COM_initASM();
	SREG = sreg;
}

void LEFT_putc(uint8_t c)
{
	uint8_t sreg = SREG;
	cli();
	LEFT_putcASM(c);
	SREG = sreg;
}

uint8_t LEFT_getc(void)
{
	uint8_t ret;
	uint8_t sreg = SREG;
	cli();
	ret = LEFT_getcASM();
	SREG = sreg;
	return ret;
}

void RIGHT_putc(uint8_t c)
{
	uint8_t sreg = SREG;
	cli();
	RIGHT_putcASM(c);
	SREG = sreg;
}

uint8_t RIGHT_getc(void)
{
	uint8_t ret;
	uint8_t sreg = SREG;
	cli();
	ret = RIGHT_getcASM();
	SREG = sreg;
	return ret;
}

void UP_putc(uint8_t c)
{
	uint8_t sreg = SREG;
	cli();
	UP_putcASM(c);
	SREG = sreg;
}

uint8_t UP_getc(void)
{
	uint8_t ret;
	uint8_t sreg = SREG;
	cli();
	ret = UP_getcASM();
	SREG = sreg;
	return ret;
}

void DOWN_putc(uint8_t c)
{
	uint8_t sreg = SREG;
	cli();
	DOWN_putcASM(c);
	SREG = sreg;
}

uint8_t DOWN_getc(void)
{
	uint8_t ret;
	uint8_t sreg = SREG;
	cli();
	ret = DOWN_getcASM();
	SREG = sreg;
	return ret;
}

//destination: WEST, EAST, NORTH, SOUTH
//c: char to be send
void COM_putc(uint8_t destination, uint8_t c)
{
	void (*ptr) (uint8_t data);
	
	if(destination == WEST)
	{
		ptr = LEFT_putc;
	}
	else if(destination == EAST)
	{
		ptr = RIGHT_putc;
	}
	else if(destination == NORTH)
	{
		ptr = UP_putc;
	}
	else if(destination == SOUTH)
	{
		ptr = DOWN_putc;
	}
	
	ptr(c);
}

//source: WEST, EAST, NORTH, SOUTH
uint8_t COM_getc(uint8_t source)
{
	uint8_t c;
	uint8_t d;
	
	uint8_t (*ptr) (void);
	
	if(source == WEST)
	{
		ptr = LEFT_getc;
	}
	else if(source == EAST)
	{
		ptr = RIGHT_getc;
	}
	else if(source == NORTH)
	{
		ptr = UP_getc;
	}
	else if(source == SOUTH)
	{
		ptr = DOWN_getc;
	}
	
	c = ptr();
	if(c == 0xF0)
	{
		d = ptr();
		return d;
	}
	else if(c == 0xA0)
	{
		//startBootloader();
	}
	
	return 0x00;
}

