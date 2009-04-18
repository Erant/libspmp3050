

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

void enable_interrupts(){
	__asm__ __volatile__(
		"			MRS r0, CPSR\n"
		"			AND r0, #0xFFFFFF3F\n"
		"			MSR CPSR, r0\n"
		:
		:
		: "r0");
}

void disable_interrupts(){
	__asm__ __volatile__(
		"			MRS r0, CPSR\n"
		"			ORR r0, #0xC0\n"
		"			MSR CPSR, r0\n"
		:
		:
		: "r0");
}