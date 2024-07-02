#ifndef _KEYBOARD_H_
#define _KEYBOARD_H_

#include "types.h"

// all necessary functions
void keyboard_initialization();
void handle_keyboard(); 

typedef struct new_terminal_t {
    uint8_t keyb_buffer_terminal[128];
    char screen_save[2000];
    uint32_t screen_x_terminal;
    uint32_t screen_y_terminal;
    uint32_t vidmem_address_terminal;
    uint32_t process_id;
    // uint32_t flag_terminal;
} new_terminal_t;

// global variables
volatile int enter_flag;
volatile int character_count;
extern uint8_t keyb_buffer[128];

new_terminal_t terminal_array[3];
int terminal_id;

#endif


