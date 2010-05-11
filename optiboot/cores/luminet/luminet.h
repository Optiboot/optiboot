#ifndef _LUMINET_H_
#define _LUMINET_H_


//#define IS_VECTOR_NODE 1

//#define VECTOR_MESSAGE 0x04

#define EEPROM_VECTOR_ADDRESS 228

#include <avr/io.h>
#include <avr/boot.h>
#include <avr/pgmspace.h>
#include <avr/interrupt.h>
#include <util/delay_basic.h>


//define this if you want to see the result of the topologyscan (blinking for each SNEW direction)
#define SHOW_TOPOLOGY_RESULT 1

#define MAX_PAGENUMBER 87

#define PAYLOAD_FLASH_ADDRESS 0

///////MAGIC KEYS
#define START 0xA0
#define RESET 0xA1
#define DATA 0xB0
#define RETRANSMISSION 0xB1
#define END 0xC0

#define RED_ON (PORTA  |= (1 << RED_LED) ) 
#define RED_ON (PORTA  |= (1 << RED_LED) ) 
#define GREEN_ON (PORTA  |= (1 << GREEN_LED) )
#define BLUE_ON (PORTA  |= (1 << BLUE_LED) )

#define RED_OFF (PORTA  &= ~(1 << RED_LED) ) 
#define GREEN_OFF (PORTA  &= ~(1 << GREEN_LED) )
#define BLUE_OFF (PORTA  &= ~(1 << BLUE_LED) )

#define RED   (1<<PA4)
#define GREEN (1<<PA5)
#define BLUE  (1<<PA6)
#define WHITE (RED | GREEN | BLUE)

#define RED_LED PA4
#define GREEN_LED PA5
#define BLUE_LED PA6

#define RED_ON (PORTA  |= (1 << RED_LED) ) 
#define RED_ON (PORTA  |= (1 << RED_LED) ) 
#define GREEN_ON (PORTA  |= (1 << GREEN_LED) )
#define BLUE_ON (PORTA  |= (1 << BLUE_LED) )
#define WHITE_ON (PORTA |= WHITE)

#define RED_OFF (PORTA  &= ~(1 << RED_LED) ) 
#define GREEN_OFF (PORTA  &= ~(1 << GREEN_LED) )
#define BLUE_OFF (PORTA  &= ~(1 << BLUE_LED) )
#define WHITE_OFF (PORTA &= ~WHITE)


#define WEST 0
#define EAST 1
#define NORTH 2
#define SOUTH 3
#define DESTINATION_WEST 0
#define DESTINATION_EAST 1
#define DESTINATION_NORTH 2
#define DESTINATION_SOUTH 3
#define DESTINATION_GREEN 4
#define DESTINATION_RED 5
#define DESTINATION_BLUE 6


#ifndef BOOTADR
#define BOOTADR 0x1600
#endif

#define	LEFT_TXDDR DDRA
#define	LEFT_TXPORT PORTA
#define	LEFT_TXPIN PINA
#define LEFT_TXPINNumber  PA2			
		
#define	LEFT_RXDDR  DDRA
#define	LEFT_RXPORT PORTA
#define	LEFT_RXPIN  PINA
#define	LEFT_RXPINNumber PA3			

#define	RIGHT_TXDDR DDRB
#define	RIGHT_TXPORT PORTB
#define	RIGHT_TXPIN PINB
#define RIGHT_TXPINNumber  PB1			
		
#define	RIGHT_RXDDR  DDRB
#define	RIGHT_RXPORT PORTB
#define	RIGHT_RXPIN  PINB
#define	RIGHT_RXPINNumber PB0			
	

#define	UP_TXDDR DDRB
#define	UP_TXPORT PORTB
#define	UP_TXPIN PINB
#define UP_TXPINNumber  PB2			
		
#define	UP_RXDDR  DDRA
#define	UP_RXPORT PORTA
#define	UP_RXPIN  PINA
#define	UP_RXPINNumber PA7

#define	DOWN_TXDDR DDRA
#define	DOWN_TXPORT PORTA
#define	DOWN_TXPIN PINA
#define DOWN_TXPINNumber  PA1			
		
#define	DOWN_RXDDR  DDRA
#define	DOWN_RXPORT PORTA
#define	DOWN_RXPIN  PINA
#define	DOWN_RXPINNumber PA0			

#define NORTH_TX (1<<PB2)
#define NORTH_RX (1<<PA7)
#define EAST_TX  (1<<PB1)
#define EAST_RX  (1<<PB0)
#define SOUTH_TX (1<<PA1)
#define SOUTH_RX (1<<PA0)
#define WEST_TX  (1<<PA2)
#define WEST_RX  (1<<PA3)

#define NORTH_LOW (PORTB &= ~NORTH_TX)
#define  EAST_LOW (PORTB &= ~EAST_TX)
#define SOUTH_LOW (PORTA &= ~SOUTH_TX)
#define  WEST_LOW (PORTA &= ~WEST_TX)

#define NORTH_HIGH (PORTB |= NORTH_TX)
#define  EAST_HIGH (PORTB |= EAST_TX)
#define SOUTH_HIGH (PORTA |= SOUTH_TX)
#define  WEST_HIGH (PORTA |= WEST_TX)

#define GET_NORTH (PINA & NORTH_RX)
#define GET_EAST  (PINB & EAST_RX)
#define GET_SOUTH (PINA & SOUTH_RX)
#define GET_WEST  (PINA & WEST_RX)

#define NORTH_PULLUP (PORTA |= NORTH_RX)
#define EAST_PULLUP (PORTB |= EAST_RX)
#define SOUTH_PULLUP (PORTA |= SOUTH_RX)
#define WEST_PULLUP (PORTA |= WEST_RX)



#endif

