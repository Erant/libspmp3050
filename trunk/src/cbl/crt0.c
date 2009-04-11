#include "gpio.h"

void main() __attribute__((noreturn));
void _start() __attribute__((noreturn));

void _start(){
	GPIO_Init();
	GPIO_UnitOn();
	main();
}