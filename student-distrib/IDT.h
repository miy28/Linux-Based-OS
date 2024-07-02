#include "lib.h"
#ifndef _IDT_H
#define _IDT_H

#ifndef ASM


// Handlers for each of the exeptions
void divide_error();
void reserved_error();
void NMI_interrupt();
void b_point();
void o_flow();
void bound_range_exceeded();
void invalid_opcode();
void dev_not_available();
void double_fault();
void coprocessor_seg_overrun();
void invalid_tss();
void seg_not_present();
void stack_seg_fault();
void general_protection();
void page_fault();
void floating_point_error();
void alignment_check();
void machine_check();
void SIMD_exception();
void system_call();
void keyboard_interrupt();
void RTC_interrupt();



#endif //ASM

#endif /* _IDT_H */
