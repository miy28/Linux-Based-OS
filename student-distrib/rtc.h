#include "lib.h"
#include "types.h"
#include "i8259.h"
#ifndef _RTC_H
#define _RTC_H

#ifndef ASM

int flag; //Flag triggered when interrupt recieved

// Initializes RTC
void rtc_init();

//Handles Interrupt from RTC
void rtc_handler();

//Opens RTC in filesystem
int32_t RTC_open(const uint8_t *filename);

//Closes RTC in filesystem
int32_t RTC_close();

//Change Frequency of RTC
int32_t RTC_write(int32_t fd, const void* buf, int32_t nbytes);

//Wait till RTC interrupt recieved
int32_t RTC_read();

#endif //ASM

#endif /* _IDT_H */
