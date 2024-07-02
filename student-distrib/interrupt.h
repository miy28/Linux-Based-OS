#ifndef INTERRUPT_H
#define INTERRUPT_H

#include "IDT.h"


#ifndef ASM

//Handlers for the interrupts
extern void keyboard_assembly();
extern void RTC_assembly();
extern void sys_call_assembly();

#endif /* ASM */

#endif /* INTERRUPT_H */
