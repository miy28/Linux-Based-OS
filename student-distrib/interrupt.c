#include "interrupt.h"
#include "IDT.h"


// HAndlers for the interrupts we didnt end up using, these are just tests that arent called
void keyboard_interrupt() {
	printf("Keyboard interrupt");
}

void RTC_interrupt() {
	printf("RTC interrupt");
}
