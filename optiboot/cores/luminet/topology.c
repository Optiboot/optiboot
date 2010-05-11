#include "topology.h"

//#define BOOTADR 0x1600


#ifndef EEPROM_VECTOR_ADDRESS
#define EEPROM_VECTOR_ADDRESS 228
#endif

//void (*startBootloader)( void ) = (void *)BOOTADR;  // pointer to the bootloader start address

//performs a topology scan and vector detection 
//returns: connections variable: SNEWSNEW - first nibble is vector path, second nibble is connection indicator
//this function only returns connections if no vector node is present
//otherwise it loops forever, sending the vector_message
void topology_scan(uint8_t *connections)
{

	uint16_t z;
	//uint8_t connections;//bit7..0: SNEWSNEW - first nibble says: vector present, second says: normal neighbour present
	
	uint8_t c;
	uint8_t d;
	
	uint8_t r;
	
	uint8_t destination;
	
	uint8_t address = EEPROM_VECTOR_ADDRESS;
	
	void (*ptr_putc) (uint8_t data);
	uint8_t (*ptr_getc) (void);
	
	//CLKPR = 0x80;//set CLKPCE bit -> next step: set prescaler
	//CLKPR = 0x02;//prescaler=2 --> 2MHz
		
	*connections = 0;

	//activate outputs, rise pull-ups. set outputs to high level
	DDRA = 0x76;
	PORTA = 0x89;
	DDRB= 0x06;
	PORTB = 0x01;
	
	//OUTPUTS TO LOW
	PORTA &= (uint8_t)~(0x06);
	PORTB &= (uint8_t)~(0x06);
	
	
	//say that the cable is ok, before we have a burning man...
	RED_ON;
	
	
  

		
		_delay_loop_2(62510); // 250ms
   
   
   for(z=0;z<250;z++)//check for connections
   {
		if(! (PINA & (1<<PA3) ) )
		{	
			*connections |= (1 << WEST);
		}
		
		if(! (PINA & (1<<PA7) ) )
		{	
			*connections |= (1 << NORTH);
		}
		
		if(! (PINA & (1<<PA0) ) )
		{	
			*connections |= (1 << SOUTH);
		}
		
		if(! (PINB & (1<<PB0) ) )
		{	
			*connections |= (1 << EAST);
		}
		_delay_loop_2(250); //1ms
	}//loop: 250*1ms = 250ms

   _delay_loop_2(62510); // 250ms
	   
	//OUTPUTS NOW HAVE BEEN LOW FOR 3*250ms = 750ms
		
		
		#ifdef IS_VECTOR_NODE
		//OUTPUTS TO LOW
		PORTA &= (uint8_t)~(0x06);
		PORTB &= (uint8_t)~(0x06);
	#else 
		//OUTPUTS HIGH
		PORTA |= (0x06);
		PORTB |= (0x06);
		
	#endif

	   _delay_loop_2(62510); // 250ms
	   _delay_loop_2(62510); // 250ms
	   
	   
	   
	#ifndef IS_VECTOR_NODE   
	   	
	   	for(z=0; z<500; z++)
	   {
		
			if(! (PINA & (1<<PA3) ) )
			{	
				if(*connections & (1 << WEST) )
				{
					//REMAINING OUTPUTS TO LOW
					PORTA &= (uint8_t)~(0x02);
					PORTB &= (uint8_t)~(0x06);				
					*connections |= (16 << WEST);
				}
			}
			if(! (PINA & (1<<PA7) ) )
			{	
				if(*connections & (1 << NORTH) )
				{			
					//REMAINING OUTPUTS TO LOW
					PORTA &= (uint8_t)~(0x06);
					PORTB &= (uint8_t)~(0x02);
					*connections |= (16 << NORTH);
				}
			}
			if(! (PINA & (1<<PA0) ) )
			{	
				if(*connections & (1 << SOUTH) )
				{				
					//REMAINING OUTPUTS TO LOW
					PORTA &= (uint8_t)~(0x04);
					PORTB &= (uint8_t)~(0x06);					
					*connections |= (16 << SOUTH);
				}
			}
			if(! (PINB & (1<<PB0) ) )
			{	
				if(*connections & (1 << EAST) )
				{							
					//REMAINING OUTPUTS TO LOW
					PORTA &= (uint8_t)~(0x06);
					PORTB &= (uint8_t)~(0x04);
					*connections |= (16 << EAST);
				}
			}
		
			_delay_loop_2(250); // 1ms
		}
		
		
		//only one vector path allowed
		if(*connections & (16 << WEST) )
		{
			*connections &= (uint8_t)~(16 << SOUTH);
			*connections &= (uint8_t)~(16 << NORTH);
			*connections &= (uint8_t)~(16 << EAST);
		}
		else if(*connections & (16 << NORTH) )
		{
			*connections &= (uint8_t)~(16 << SOUTH);
			*connections &= (uint8_t)~(16 << WEST);
			*connections &= (uint8_t)~(16 << EAST);
		}
		else if(*connections & (16 << SOUTH) )
		{
			*connections &= (uint8_t)~(16 << NORTH);
			*connections &= (uint8_t)~(16 << WEST);
			*connections &= (uint8_t)~(16 << EAST);
		}
		else if(*connections & (16 << EAST) )
		{
			*connections &= (uint8_t)~(16 << SOUTH);
			*connections &= (uint8_t)~(16 << WEST);
			*connections &= (uint8_t)~(16 << NORTH);
		}
	   	
		#else
	   //Vector node does not loop so we have to add the 500ms loop duration here
	   _delay_loop_2(62510); // 250ms
	   _delay_loop_2(62510); // 250ms

	#endif
	
	RED_OFF;
	GREEN_OFF;
	BLUE_OFF;

		
	   _delay_loop_2(62510); // 250ms
	   _delay_loop_2(62510); // 250ms

	//tx lines now have been HIGH (normal node) / LOW (vector node) for 1500ms

		//tx lines high again
		PORTA |= (0x06);
		PORTB |= (0x06);
			
		_delay_loop_2(62510); // 250ms
			
		
#ifdef SHOW_TOPOLOGY_RESULT
		//show the result of the topology scan by blinking LEDs
		for(z=0;z<4;z++)
		{
			if(*connections & (16 << z) )
			{
				BLUE_ON;//a connection to a node on the vector path
				
			}
			else if(*connections & (1 << z) )
			{
				GREEN_ON;//a simple connection to another node
			}
			else
			{
				RED_ON;//no connection in this direction
			}
			
			_delay_loop_2(62510); // 250ms
			_delay_loop_2(62510); // 250ms
			RED_OFF;
			GREEN_OFF;
			BLUE_OFF;
		   _delay_loop_2(62510); // 250ms
		   _delay_loop_2(62510); // 250ms
		   	
		}
#endif
	
	//function only returns connections if no vector node is present
	//otherwise it starts the bootloader
	

	eeprom_write_byte(address, *connections);//write vector message to eeprom
	_delay_loop_2(400);
	
	if(*connections >=16)
	{
		cli();
		//startBootloader();
		//asm("call 0x1600");
	}
		



}

//If a vector is present, it sends a message to the vector path. This message is a single byte
//the byte is stored in the eeprom at address 228 and returned by this function
/*
uint8_t topology_getVectorMessage(void)
{
	uint8_t *address = EEPROM_VECTOR_ADDRESS; //adress to read from
	uint8_t data;
	data = eeprom_read_byte(address);
	return data;
	
	
	


}
*/