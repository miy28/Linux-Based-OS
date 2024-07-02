/* kernel.c - the C part of the kernel
 * vim:ts=4 noexpandtab
 */

#include "multiboot.h"
#include "x86_desc.h"
#include "lib.h"
#include "i8259.h"
#include "debug.h"
#include "tests.h"
#include "IDT.h"
#include "interrupt.h"
#include "keyboard.h"
#include "0x86_interrupt.h"
#include "paging.h"
#include "filesystem.h"
#include "rtc.h"
#include "syscall.h"

#define RUN_TESTS

/* Macros. */
/* Check if the bit BIT in FLAGS is set. */
#define CHECK_FLAG(flags, bit)   ((flags) & (1 << (bit)))

/* Check if MAGIC is valid and print the Multiboot information structure
   pointed by ADDR. */
void entry(unsigned long magic, unsigned long addr) {

    multiboot_info_t *mbi;

    /* Clear the screen. */
    clear();

    /* Am I booted by a Multiboot-compliant boot loader? */
    if (magic != MULTIBOOT_BOOTLOADER_MAGIC) {
        printf("Invalid magic number: 0x%#x\n", (unsigned)magic);
        return;
    }

    /* Set MBI to the address of the Multiboot information structure. */
    mbi = (multiboot_info_t *) addr;

    /* Print out the flags. */
    printf("flags = 0x%#x\n", (unsigned)mbi->flags);

    /* Are mem_* valid? */
    if (CHECK_FLAG(mbi->flags, 0))
        printf("mem_lower = %uKB, mem_upper = %uKB\n", (unsigned)mbi->mem_lower, (unsigned)mbi->mem_upper);

    /* Is boot_device valid? */
    if (CHECK_FLAG(mbi->flags, 1))
        printf("boot_device = 0x%#x\n", (unsigned)mbi->boot_device);

    /* Is the command line passed? */
    if (CHECK_FLAG(mbi->flags, 2))
        printf("cmdline = %s\n", (char *)mbi->cmdline);

    if (CHECK_FLAG(mbi->flags, 3)) {
        int mod_count = 0;
        int i;
        module_t* mod = (module_t*)mbi->mods_addr;
        while (mod_count < mbi->mods_count) {
            filesystem_addr = (unsigned int)mod->mod_start;
            printf("Module %d loaded at address: 0x%#x\n", mod_count, (unsigned int)mod->mod_start);
            printf("Module %d ends at address: 0x%#x\n", mod_count, (unsigned int)mod->mod_end);
            printf("First few bytes of module:\n");
            for (i = 0; i < 16; i++) {
                printf("0x%x ", *((char*)(mod->mod_start+i)));
            }
            printf("\n");
            mod_count++;
            mod++;
        }
    }
    /* Bits 4 and 5 are mutually exclusive! */
    if (CHECK_FLAG(mbi->flags, 4) && CHECK_FLAG(mbi->flags, 5)) {
        printf("Both bits 4 and 5 are set.\n");
        return;
    }

    /* Is the section header table of ELF valid? */
    if (CHECK_FLAG(mbi->flags, 5)) {
        elf_section_header_table_t *elf_sec = &(mbi->elf_sec);
        printf("elf_sec: num = %u, size = 0x%#x, addr = 0x%#x, shndx = 0x%#x\n",
                (unsigned)elf_sec->num, (unsigned)elf_sec->size,
                (unsigned)elf_sec->addr, (unsigned)elf_sec->shndx);
    }

    /* Are mmap_* valid? */
    if (CHECK_FLAG(mbi->flags, 6)) {
        memory_map_t *mmap;
        printf("mmap_addr = 0x%#x, mmap_length = 0x%x\n",
                (unsigned)mbi->mmap_addr, (unsigned)mbi->mmap_length);
        for (mmap = (memory_map_t *)mbi->mmap_addr;
                (unsigned long)mmap < mbi->mmap_addr + mbi->mmap_length;
                mmap = (memory_map_t *)((unsigned long)mmap + mmap->size + sizeof (mmap->size)))
            printf("    size = 0x%x, base_addr = 0x%#x%#x\n    type = 0x%x,  length    = 0x%#x%#x\n",
                    (unsigned)mmap->size,
                    (unsigned)mmap->base_addr_high,
                    (unsigned)mmap->base_addr_low,
                    (unsigned)mmap->type,
                    (unsigned)mmap->length_high,
                    (unsigned)mmap->length_low);
    }

    /* Construct an LDT entry in the GDT */
    {
        seg_desc_t the_ldt_desc;
        the_ldt_desc.granularity = 0x0;
        the_ldt_desc.opsize      = 0x1;
        the_ldt_desc.reserved    = 0x0;
        the_ldt_desc.avail       = 0x0;
        the_ldt_desc.present     = 0x1;
        the_ldt_desc.dpl         = 0x0;
        the_ldt_desc.sys         = 0x0;
        the_ldt_desc.type        = 0x2;

        SET_LDT_PARAMS(the_ldt_desc, &ldt, ldt_size);
        ldt_desc_ptr = the_ldt_desc;
        lldt(KERNEL_LDT);
    }

    /* Construct a TSS entry in the GDT */
    {
        seg_desc_t the_tss_desc;
        the_tss_desc.granularity   = 0x0;
        the_tss_desc.opsize        = 0x0;
        the_tss_desc.reserved      = 0x0;
        the_tss_desc.avail         = 0x0;
        the_tss_desc.seg_lim_19_16 = TSS_SIZE & 0x000F0000;
        the_tss_desc.present       = 0x1;
        the_tss_desc.dpl           = 0x0;
        the_tss_desc.sys           = 0x0;
        the_tss_desc.type          = 0x9;
        the_tss_desc.seg_lim_15_00 = TSS_SIZE & 0x0000FFFF;

        SET_TSS_PARAMS(the_tss_desc, &tss, tss_size);

        tss_desc_ptr = the_tss_desc;

        tss.ldt_segment_selector = KERNEL_LDT;
        tss.ss0 = KERNEL_DS;
        tss.esp0 = 0x800000;
        ltr(KERNEL_TSS);
    }

    /* Init IDT*/
    int i;
    for (i=0;i<NUM_VEC;i++) {
        if (i<20) {
            if (i!=15) {
                idt[i].present = 1; //set the present of all entries to 1 execept 1 which is reserved
            }
            else {
                idt[i].present = 0;
            }
        }
        else {
            idt[i].present = 0;
        }

        idt[i].seg_selector = KERNEL_CS; //Specifies GDT ring code is run at
        idt[i].reserved4 = 0; //Always 0
        idt[i].reserved3 = 1; //0 for exceptions, set to 1 for interrupts
        idt[i].reserved2 = 1; //always 1
        idt[i].reserved1 = 1; //Alwyas 1
        idt[i].size = 1; //Always 1 for our purposes
        idt[i].reserved0 = 0; //Always 0
        idt[i].dpl = 0; //Most occur in Kernel, system call is at user program level

    }
    //Set IDT entry for all exceptions
    SET_IDT_ENTRY(idt[0x00], divide_error);
    SET_IDT_ENTRY(idt[0x01], reserved_error);
    SET_IDT_ENTRY(idt[0x02], NMI_interrupt);
    SET_IDT_ENTRY(idt[0x03], b_point);
    SET_IDT_ENTRY(idt[0x04], o_flow);
    SET_IDT_ENTRY(idt[0x05], bound_range_exceeded);
    SET_IDT_ENTRY(idt[0x06], invalid_opcode);
    SET_IDT_ENTRY(idt[0x07], dev_not_available);
    SET_IDT_ENTRY(idt[0x08], double_fault);
    SET_IDT_ENTRY(idt[0x09], coprocessor_seg_overrun);
    SET_IDT_ENTRY(idt[0x0A], invalid_tss);
    SET_IDT_ENTRY(idt[0x0B], seg_not_present);
    SET_IDT_ENTRY(idt[0x0C], stack_seg_fault);
    SET_IDT_ENTRY(idt[0x0D], general_protection);
    SET_IDT_ENTRY(idt[0x0E], page_fault);
    SET_IDT_ENTRY(idt[0x10], floating_point_error);
    SET_IDT_ENTRY(idt[0x11], alignment_check);
    SET_IDT_ENTRY(idt[0x12], machine_check);
    SET_IDT_ENTRY(idt[0x13], SIMD_exception);

    // IDT entry for system calls
    SET_IDT_ENTRY(idt[0x80], sys_call_assembly);
    idt[0x80].present = 1;
    idt[0x80].dpl = 3; //Low security

    // IDT for Interrupts
    SET_IDT_ENTRY(idt[0x21], keyboard_assembly);
    idt[0x21].reserved3 = 0;
    idt[0x21].present = 1;
    SET_IDT_ENTRY(idt[0x28], RTC_assembly);
    idt[0x28].reserved3 = 0;
    idt[0x28].present = 1;
    idt[0x28].dpl = 3;
    

    /* Init the PIC/ keyboard/ RTC */
    i8259_init();

    keyboard_initialization();

    rtc_init();

    clear();

    filesystem_init();

    file_op_table_init();

    paging_init();
    
    terminal_new_init();

    /* Initialize devices, memory, filesystem, enable device interrupts on the
     * PIC, any other initialization stuff... */

    /* Enable interrupts */
    /* Do not enable the following until after you have set up your
     * IDT correctly otherwise QEMU will triple fault and simple close
     * without showing you any output */
    //printf("Enabling Interrupts\n");

    sti();

#ifdef RUN_TESTS
    /* Run tests */
    launch_tests();
#endif
    /* Execute the first program ("shell") ... */

    /* Spin (nicely, so we don't chew up cycles) */
    asm volatile (".1: hlt; jmp .1;");
}
