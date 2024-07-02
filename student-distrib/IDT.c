#include "IDT.h"
#include "0x86_interrupt.h"
#include "syscall.h"

// All these functions are the same, they just need to print the specific exceptio triggered and loop infinitely

void divide_error() {
	printf("Exception: Divide by zero\n");
	sys_halt(0x00);
	while (1) {}
}
void reserved_error() {
	printf("Exception: Reserved for intel\n");
	sys_halt(0x01);
	while (1) {}
}
void NMI_interrupt() {
	printf("Exception: Nonmaskable external interrupt\n");
	sys_halt(0x02);
	while (1) {}
}
void b_point() {
	printf("Exception: Reached Breakpoint\n");
	sys_halt(0x03);
	while (1) {}
}
void o_flow() {
	printf("Exception: Overflow\n");
	sys_halt(0x04);
	while (1) {}
}
void bound_range_exceeded() {
	printf("Exception: BOUND range exceeded\n");
	sys_halt(0x05);
	while (1) {}
}
void invalid_opcode() {
	printf("Exception: Invalid opcode\n");
	sys_halt(0x06);
	while (1) {}
}
void dev_not_available() {
	printf("Exception: Device not available (math coprocessor)\n");
	sys_halt(0x07);
	while (1) {}
}
void double_fault() {
	printf("Exception: Double fault\n");
	sys_halt(0x08);
	while (1) {}
}
void coprocessor_seg_overrun() {
	printf("Exception: Coprocessor segment overrun (reserved)\n");
	sys_halt(0x09);
	while (1) {}
}
void invalid_tss() {
	printf("Exception: invalid TSS\n");
	sys_halt(0x0A);
	while (1) {}
}
void seg_not_present() {
	printf("Exception: Segment not Present\n");
	sys_halt(0x0B);
	while (1) {}
}
void stack_seg_fault() {
	printf("Exception: stack segment fault\n");
	sys_halt(0x0C);
	while (1) {}
}
void general_protection() {
	printf("Exception: General Protection\n");
	sys_halt(0x0D);
	while (1) {}
}
void page_fault() {
	printf("Exception: Page Fault\n");
	sys_halt(0x0E);
	while (1) {}
}
void floating_point_error() {
	printf("Exception: Floating point error\n");
	sys_halt(0x10);
	while (1) {}
}
void alignment_check() {
	printf("Exception: Alignment check\n");
	sys_halt(0x11);
	while (1) {}
}
void machine_check() {
	printf("Exception: Machine check\n");
	sys_halt(0x12);
	while (1) {}
}
void SIMD_exception() {
	printf("Exception: SIMD Floating point exception\n");
	sys_halt(0x13);
	while (1) {}
}
void system_call() {
	printf("System Call\n");
	while (1) {}
}

