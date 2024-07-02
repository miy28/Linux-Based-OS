#define ASM     1

# Push all registers and flags to the stack before calling the c functions,
# and then recovering the values from the stack
.globl keyboard_assembly, RTC_assembly, sys_call_assembly

    keyboard_assembly:
    pushal 
    pushfl
    call handle_keyboard
    popfl
    popal
    iret

    RTC_assembly:
    pushal
    pushfl
    call rtc_handler
    popfl
    popal
    iret

    sys_call_assembly:
    cmpl	$0, %eax
    jle     invalid_call

    cmpl	$11, %eax
    jge     invalid_call
    pushfl
    pushl   %esp
    pushl   %ebp
    pushl   %edi
    pushl   %esi

    pushl   %edx
    pushl   %ecx
    pushl   %ebx
    
    call	*jump_table(,%eax,4)

    popl    %ebx
    popl    %ecx
    popl    %edx

    popl    %esi
    popl    %edi
    popl    %ebp
    popl    %esp
    popfl
    iret

invalid_call:
    movl $-1, %eax
    iret


jump_table:
	.long 0x0, sys_halt, sys_execute, sys_read, sys_write, sys_open, sys_close, sys_getargs, sys_vidmap, sys_set_handler, sys_sigreturn
    
