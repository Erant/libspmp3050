#include "pwr.h"

void main() __attribute__((noreturn));
void _start() __attribute__((noreturn));

void _start(){
	PWR_UnitOn();
	main();
}