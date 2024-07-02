#include "rtc.h"




// From os dev
/* void rtc_init();
 * Inputs: None
 * Return Value: void
 *  Function: Initializes the RTC */
void rtc_init()
{
    char first;
    outb(0x8B, 0x70);		// select register B, and disable NMI
    first=inb(0x71);	// read the current value of register B
    outb(0x8B, 0x70);		// set the index again (a read will reset the index to register D)
    first = first | 0x40;
    outb(first, 0x71);	// write the previous value ORed with 0x40. This turns on bit 6 of register B

    int rate = 0x02;
    rate &= 0x0F;			// rate must be above 2 and not over 15
    outb(0x8A, 0x70);		// set index to register A, disable NMI
    first=inb(0x71);	// get initial value of register A
    outb(0x8A, 0x70);		// reset index to A
    outb((first & 0xF0) | rate, 0x71); //write only our rate to A. Note, rate is the bottom 4 bits.

    enable_irq(8);
    outb(0x8C, 0x70);
    inb(0x71);
}

/* void rtc_handler();
 * Inputs: None
 * Return Value: void
 *  Function: Handles the recieved interrupt signal and resets the RTC/ PIC */
void rtc_handler() 
{
    cli(); //Stop interrupts
    //test_interrupts();
	flag = 1;
    outb(0x8C, 0x70); //Register B
    inb(0x71);         //Read from B
    send_eoi(8);        //Send EOI to correct port
    sti(); //Reset interrupts 
}

/* void RTC_open();
 * Inputs: filename: name of file in filesystem
 * Return Value: 0 if successful
 *  Function: Opens the RTC and initializes it to a value */
int32_t RTC_open(const uint8_t *filename) {
	int rate = 0x02;        // Default value
    rate &= 0x0F;			// Mask it
    outb(0x8A, 0x70);		// set index to register A, disable NMI
    char first=inb(0x71);	// get initial value of register A
    outb(0x8A, 0x70);		// reset index to A
    outb((first & 0xF0) | rate, 0x71); //write only our rate to A. Note, rate is the bottom 4 bits.
	return 0;
}

/* void RTC_close();
 * Inputs: none
 * Return Value: 0 if successful
 *  Function: Closes the RTC */
int32_t RTC_close() {
	return 0;
}

/* void RTC_write();
 * Inputs: freq: Frequency the RTC should be set to, must be a power of 2 less than or equal to 1024
 * Return Value: 0 if successful
 *  Function: Sets the RTC to the given frequency */
int32_t RTC_write(int32_t fd, const void* buf, int32_t nbytes) {
	char first;
	int rate;
    // Conversion from freq to rate from osdev wiki: frequency =  32768 >> (rate-1);
	int as_int;
	memcpy(&as_int, buf, nbytes);
	switch(as_int) {
		case 1024:
			rate = 0x06;
			break;
		case 512:
			rate = 0x07;
			break;
		case 256:
			rate = 0x08;
			break;
		case 128:
			rate = 0x09;
			break;
		case 64:
			rate = 0x0A;
			break;
		case 32:
			rate = 0x0B;
			break;
		case 16:
			rate = 0x0C;
			break;
		case 8:
			rate = 0x0D;
			break;
		case 4:
			rate = 0x0E;
			break;
		case 2:
			rate = 0x0F;
			break;
		default:
		return 0;
	}
    rate &= 0x0F;			// Mask it
    outb(0x8A, 0x70);		// set index to register A, disable NMI
    first=inb(0x71);	// get initial value of register A
    outb(0x8A, 0x70);		// reset index to A
    outb((first & 0xF0) | rate, 0x71); //write only our rate to A. Note, rate is the bottom 4 bits.
	return 0;
}

/* void RTC_read();
 * Inputs: none
 * Return Value: 0 if successful
 *  Function: waits until RTC sends an interrupt */
int32_t RTC_read() {
	flag = 0; //Set the flag
	while (1) {//spin
		if (flag == 1) {//break when flag is set
			return 0;
		}
	}
	return 0;
}
