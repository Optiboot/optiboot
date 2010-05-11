//void (*startBootloader)( void ) = (void(*)(void))(0x1600);




int main(void)
{
	//startBootloader();
	
	
	mainconnections = 0;
	topology_scan(&mainconnections);//luminet needs to perform topology scan at the beginning of every payload
	
	//cli();
	//CLKPR = (1 << CLKPCE);
	//CLKPR = 0x00;//8MHz
	
		if(mainconnections >= 16)
		{
			//startBootloader();
				__asm__ __volatile
			 (
			      "ldi r30,0x00"  "\n\t"
			      "ldi r31,0x0b"  "\n\t"
			      "icall" "\n\t"
			 );
		}
	
		
	init();

	setup();
    
	for (;;)
		loop();
        
	return 0;
}

