

// Delay loop taken from original firmware.
void delay_ms(int ms){
	for(volatile int i = 0; i < ms; i++)
		for(volatile int j = 0; j < 255; j++);
}

// Delay loop taken from original firmware.
void delay_us(int us){
	for(volatile int i = 0; i < us; i++)
		for(volatile int j = 0; j < 69; j++);
}