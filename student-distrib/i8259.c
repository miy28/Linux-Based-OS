/* i8259.c - Functions to interact with the 8259 interrupt controller
 * vim:ts=4 noexpandtab
 */

#include "i8259.h"
#include "lib.h"

/* Interrupt masks to determine which interrupts are enabled and disabled */
static uint8_t master_mask = 0xFF; /* IRQs 0-7  */
static uint8_t slave_mask = 0xFF;  /* IRQs 8-15 */

/* Initialize the 8259 PIC */

/* void i8259_init(void);
 * Inputs: void
 * Return Value: none
 * Function: initializes the pic */
void i8259_init(void) {
    outb(ICW1, MASTER_8259_PORT_0); // master init
    outb(ICW2_MASTER, MASTER_8259_PORT_1); // registers 0-7
    outb(ICW3_MASTER, MASTER_8259_PORT_1); // slave on the second reg

    outb(ICW4, MASTER_8259_PORT_1); //8086 mode

    outb(ICW1, SLAVE_8259_PORT_0); //slave init
    outb(ICW2_SLAVE, SLAVE_8259_PORT_1); // registers 0-7
    outb(ICW3_SLAVE, SLAVE_8259_PORT_1); // master's slave 2
 
    outb(ICW4, SLAVE_8259_PORT_1); //8086 mode

    outb(master_mask, MASTER_8259_PORT_1); 
    outb(slave_mask, SLAVE_8259_PORT_1); // masking done at the end
    enable_irq(2);
}


/* Enable (mask) the specified IRQ */

/* void enable_irq(uint32_t irq_num)
 * Inputs: uint32_t irq_num
 * Return Value: none
 * Function:  enable a specific irq */
void enable_irq(uint32_t irq_num) {
    unsigned int mask;
    if (irq_num<=IRQ_MAX) {
        if (irq_num & 8) {
            mask = ~(1 << (irq_num - MASTER_PIC)); // masking done for master
            slave_mask &= mask;
            outb(slave_mask, SLAVE_8259_PORT_1); // done when IRQ is less than the first 8
        } 
        else {
            mask = ~(1 << irq_num);
            master_mask &= mask; // masking done for slave
            outb(master_mask, MASTER_8259_PORT_1); // done when IRQ is more than the first 8 
        }
    }
    return; // done when not in bounds
}

/* Disable (mask) the specified IRQ */

/* void disable_irq(uint32_t irq_num)
 * Inputs: uint32_t irq_num
 * Return Value: none
 * Function:  disable a specific irq */
void disable_irq(uint32_t irq_num) {
    unsigned int mask;
    if (irq_num<=IRQ_MAX) {
        if (irq_num & 8) {
            mask = (1 << (irq_num - MASTER_PIC));
            slave_mask |= mask;
            outb(slave_mask, SLAVE_8259_PORT_1);
        } 
        else {
            mask = (1 << irq_num);
            master_mask |= mask;
            outb(master_mask, MASTER_8259_PORT_1);
        }
    }
    return; // similar logic tot he enable except we are using or equals and excluding the not
}

/* Send end-of-interrupt signal for the specified IRQ */

/* void send_eoi(uint32_t irq_num);
 * Inputs: uint32_t irq_num
 * Return Value: none
 * Function:  send the interrupt signal */
void send_eoi(uint32_t irq_num) {
    // Old Version
    // if (irq_num<=IRQ_MAX) {
    //     if (irq_num>=MASTER_PIC) {
    //         outb(EOI | (irq_num-IRQ_MAX), SLAVE_8259_PORT_0);
    //         outb(EOI | irq_num, MASTER_8259_PORT_0);
    //     }
    //     else {
    //         outb(EOI | irq_num, MASTER_8259_PORT_0);
    //     }
    // } // similar logic to the enable and disable but we are sending output signal EOI to right of or contents
    // return;
    
    // Version from osdev
    if (irq_num<=IRQ_MAX) {
        if (irq_num>=MASTER_PIC) {
            outb(EOI | (irq_num-8), SLAVE_8259_PORT_0); //Shift by 8 for slave
            outb(EOI | 2, MASTER_8259_PORT_0); //Port 2 for slave
        }
        outb(EOI | irq_num, MASTER_8259_PORT_0); //Master port
    }
}
