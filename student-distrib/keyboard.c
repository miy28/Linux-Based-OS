#include "terminal.h"
#include "keyboard.h"
#include "i8259.h"
#include "lib.h"
#include "syscall.h"
#include "x86_desc.h"

#define KEYBOARD_IRQ 1
#define enter 0x1C
#define backspace 0x0E
#define tab 0x0F
#define caps_pressed 0x3A
#define caps_released 0xBA
#define left_shift_pressed 0x2A
#define left_shift_released 0xAA
#define left_ctrl_pressed 0x1D
#define left_ctrl_released 0x9D
#define left_alt_pressed 0x38        
#define left_alt_released 0xB8
#define board_size 61
#define data_port 0x60
#define ctrlL 0x26


uint8_t keyb_buffer[128];
int shift_flag = 0;
int ctrl_flag = 0;
int caps_flag = 0;
int alt_flag = 0; //declared the flags here to see what kinds of keys have been pressed along with global keyb buffer


// the lower case keys
char lower_keys[board_size] =  
{
    '\0', '\0' , '1', '2', '3', '4', '5', '6',
    '7', '8', '9', '0', '-', '=', '\b', '\t',
    'q', 'w', 'e', 'r', 't', 'y', 'u', 'i',
    'o', 'p', '[',']', '\n','\0' , 'a', 's', 'd',
    'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`', '\0'
     , '\\','z' , 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', '\0', '*', '\0' , ' ', '\0'
};

// the upper case keys
char upper_keys[board_size] =  
{
    NULL, NULL , '1', '2', '3', '4', '5', '6',
    '7', '8', '9', '0', '-', '=', '\b', '\t',
    'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I',
    'O', 'P', '[',']', '\n',NULL , 'A', 'S', 'D',
    'F', 'G', 'H', 'J', 'K', 'L', ';', '\'', '`', NULL
     , '\\','Z' , 'X', 'C', 'V', 'B', 'N', 'M', ',', '.', '/', NULL, '*', NULL , ' ', NULL
};

// the shift case keys
char shift_keys[board_size] =  
{
    NULL, NULL , '!', '@', '#', '$', '%', '^',
    '&', '*', '(', ')', '_', '+', '\b', '\t',
    'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I',
    'O', 'P', '{','}', '\n',NULL , 'A', 'S', 'D',
    'F', 'G', 'H', 'J', 'K', 'L', ':', '\"', '~', NULL
     , '?','Z' , 'X', 'C', 'V', 'B', 'N', 'M', '<', '>', '?', NULL, '*', NULL , ' ', NULL
};

/* void keyboard_initialization;
 * Inputs: void
 * Return Value: none
 * Function: initializes keyboard */
void keyboard_initialization()
{
    enable_irq(KEYBOARD_IRQ);
    //enter_flag = 0;
    //character_count = 0;
} // ensures that keyboard will be able to run


/* void handle_keyboard;
 * Inputs: void
 * Return Value: none
 * Function: handles all keyboard related inputs */
void handle_keyboard()
{
    cli();
    uint8_t scanned_key;
    scanned_key = inb(data_port); // scans key
    //printf(scanned_key);
    uint32_t screen_x;
    uint32_t screen_y;

    switch(scanned_key) // scans the key that we have
    {
        case left_shift_pressed: // left shift held and released
            shift_flag = 1;
            //printf("Shift Pressed");
            break;
        case left_shift_released:
            shift_flag = 0;
            //printf("Shift Relesed");
            break;
        case left_ctrl_pressed: // left ctrl held and released
            ctrl_flag = 1;
            //printf("Control Pressed");
            break;
        case left_ctrl_released:
            ctrl_flag = 0;
            //printf("Control Relesed");
            break;
        case caps_pressed: // caps held and released
            caps_flag = !caps_flag;
            //printf("caps pressed");
            break;
        case left_alt_pressed: // left ctrl held and released
            alt_flag = 1;
            //printf("Control Pressed");
            break;
        case left_alt_released:
            alt_flag = 0;
            //printf(

        default:
        if((scanned_key > 0 && scanned_key < 58)) // key range to ensure we have valid chars
        {
            if(caps_flag) //  caps flag case for all caps
            {
                putc(upper_keys[scanned_key]);
                keyb_buffer[character_count] = upper_keys[scanned_key];
                character_count++;
            }

            else if(shift_flag) // shift flags case for when shift is held
            {
                putc(shift_keys[scanned_key]);
                keyb_buffer[character_count] = shift_keys[scanned_key];
                character_count++;
            }

            else if(ctrl_flag == 1 && (lower_keys[scanned_key] == 'l' || upper_keys[scanned_key] == 'L' || shift_keys[scanned_key] == 'L'))
            {
                clear();
                move_cursor(0,0); // ctrl L moves the cursor to the top left of screen and clears
            }

            else if (scanned_key == backspace) // backspace is moving the cursor back and replacing old char with a space
            {
                if(character_count > 0)
                {
                    character_count--; // decrements char count
                    screen_x = ret_x() - 1;
                    screen_y = ret_y(); // adjusts positioning for the screen
                    move_cursor(screen_x, screen_y); // moves cursor to approrpriate part
                    putc(' '); // puts in space
                    screen_x = ret_x() - 1; 
                    screen_y = ret_y();
                    move_cursor(screen_x, screen_y); // repeat again to finalize
                }
            }

            else if (scanned_key == tab) // backspace is moving the cursor back and replacing old char with a space
            {
                putc(' ');
                putc(' ');
                putc(' ');
                putc(' ');
                keyb_buffer[character_count] = ' ';
                keyb_buffer[character_count] = ' ';
                keyb_buffer[character_count] = ' ';
                keyb_buffer[character_count] = ' ';
                character_count = character_count + 4; // decrements char count
                screen_x = ret_x();
                screen_y = ret_y(); // adjusts positioning for the screen
                move_cursor(screen_x, screen_y); // moves cursor to approrpriate part
            }


            else
            {
                putc(lower_keys[scanned_key]);
                keyb_buffer[character_count] = lower_keys[scanned_key];
                character_count++; // default case for when nothing is pressed
                screen_x = ret_x(); 
                screen_y = ret_y();
                move_cursor(screen_x, screen_y); // repeat again to finalize
            }
        }

        if(alt_flag == 1 && scanned_key == 0x3B) // F1
        {
            memcpy((uint8_t*)(terminal_array[terminal_id].keyb_buffer_terminal), (uint8_t*)keyb_buffer, sizeof(keyb_buffer));//copy keyboard bufer
            //memcpy((uint32_t*)(terminal_array[terminal_id].vidmem_address_terminal), (uint32_t*)0xB8000, 4096);
            terminal_array[terminal_id].screen_x_terminal = ret_x();//set x and y
            terminal_array[terminal_id].screen_y_terminal = ret_y();
            int j;
            for(j = 0; j < 2000; j++){
                terminal_array[terminal_id].screen_save[j] = *(uint8_t*)(0xB8000 + (j << 1));
                *(uint8_t*)(0xB8000 + (j << 1)) = terminal_array[0].screen_save[j];
                *(uint8_t*)(0xB8000 + (j << 1) + 1) = 7;
            }

            memset(keyb_buffer, '\0', sizeof(keyb_buffer)); // clears the buffer itself
            clear();
            move_cursor(0, 0);
            for(j = 0; j < 2000; j++){
                //terminal_array[terminal_id].screen_save[j] = *(uint8_t*)(0xB8000 + (j << 1));
                *(uint8_t*)(0xB8000 + (j << 1)) = terminal_array[0].screen_save[j];
                *(uint8_t*)(0xB8000 + (j << 1) + 1) = 7;
            }

            terminal_id = 0;

            memcpy((uint32_t*)keyb_buffer, (uint8_t*)(terminal_array[0].keyb_buffer_terminal), sizeof(keyb_buffer));
            //memcpy((uint32_t*)(0xB8000), (uint32_t*)(terminal_array[0].vidmem_address_terminal), 4096);

            move_cursor(terminal_array[0].screen_x_terminal, terminal_array[0].screen_y_terminal);
            if (shell_flag==0) {//if program running isnt shell
                send_eoi(KEYBOARD_IRQ);
                send_eoi(8);
                terminal_array[0].screen_x_terminal = 0;
                for(j = 0; j < 80; j++){
                    //terminal_array[terminal_id].screen_save[j] = *(uint8_t*)(0xB8000 + (j << 1));
                    *(uint8_t*)(0xB8000 + ((j+(terminal_array[0].screen_y_terminal*80)) << 1)) = ' ';
                    *(uint8_t*)(0xB8000 + ((j+(terminal_array[0].screen_y_terminal*80)) << 1) + 1) = 7;
                }
                move_cursor(terminal_array[0].screen_x_terminal, terminal_array[0].screen_y_terminal);
                sys_halt(0);
            }
            else {
                terminal_array[0].screen_x_terminal = 7;
                //terminal_array[0].screen_y_terminal = terminal_array[0].screen_y_terminal+1;
                move_cursor(terminal_array[0].screen_x_terminal, terminal_array[0].screen_y_terminal);
                char os[] = "391OS>                                                                                               ";
                for(j = 0; j < 80; j++){
                    //terminal_array[terminal_id].screen_save[j] = *(uint8_t*)(0xB8000 + (j << 1));
                    *(uint8_t*)(0xB8000 + ((j+(terminal_array[0].screen_y_terminal*80)) << 1)) = os[j];
                    *(uint8_t*)(0xB8000 + ((j+(terminal_array[0].screen_y_terminal*80)) << 1) + 1) = 7;
                }
            }
        }

        else if(alt_flag == 1 && scanned_key == 0x3C) // F2
        {
            memcpy((uint8_t*)(terminal_array[terminal_id].keyb_buffer_terminal), (uint8_t*)keyb_buffer, sizeof(keyb_buffer));
            //memcpy((uint32_t*)(terminal_array[terminal_id].vidmem_address_terminal), (uint32_t*)0xB8000, 4096);
            terminal_array[terminal_id].screen_x_terminal = ret_x();
            terminal_array[terminal_id].screen_y_terminal = ret_y();
            int j;
            for(j = 0; j < 2000; j++){
                terminal_array[terminal_id].screen_save[j] = *(uint8_t*)(0xB8000 + (j << 1));
                *(uint8_t*)(0xB8000 + (j << 1)) = terminal_array[1].screen_save[j];
                *(uint8_t*)(0xB8000 + (j << 1) + 1) = 7;
            }

            memset(keyb_buffer, '\0', sizeof(keyb_buffer)); // clears the buffer itself
            clear();
            move_cursor(0, 0);

            for(j = 0; j < 2000; j++){
                //terminal_array[terminal_id].screen_save[j] = *(uint8_t*)(0xB8000 + (j << 1));
                *(uint8_t*)(0xB8000 + (j << 1)) = terminal_array[1].screen_save[j];
                *(uint8_t*)(0xB8000 + (j << 1) + 1) = 7;
            }

            terminal_array[terminal_id].process_id = id;
            terminal_id = 1;

            memcpy((uint32_t*)keyb_buffer, (uint8_t*)(terminal_array[1].keyb_buffer_terminal), sizeof(keyb_buffer));
            //memcpy((uint32_t*)(0xB8000), (uint32_t*)(terminal_array[1].vidmem_address_terminal), 4096);

            move_cursor(terminal_array[1].screen_x_terminal, terminal_array[1].screen_y_terminal);
            // uint8_t* arg;
            // send_eoi(KEYBOARD_IRQ);
            // arg = (uint8_t *)"shell ";
            // asm ("int $0x80" :: "a"(2), "b"(arg));
            // while (1){};
            if (shell_flag==0) {
                send_eoi(KEYBOARD_IRQ);
                send_eoi(8);
                terminal_array[1].screen_x_terminal = 0;
                for(j = 0; j < 80; j++){
                    //terminal_array[terminal_id].screen_save[j] = *(uint8_t*)(0xB8000 + (j << 1));
                    *(uint8_t*)(0xB8000 + ((j+(terminal_array[1].screen_y_terminal*80)) << 1)) = ' ';
                    *(uint8_t*)(0xB8000 + ((j+(terminal_array[1].screen_y_terminal*80)) << 1) + 1) = 7;
                }
                move_cursor(terminal_array[1].screen_x_terminal, terminal_array[1].screen_y_terminal);
                sys_halt(0);
            }
            else {
                terminal_array[1].screen_x_terminal = 7;
                //terminal_array[1].screen_y_terminal = terminal_array[1].screen_y_terminal+1;
                move_cursor(terminal_array[1].screen_x_terminal, terminal_array[1].screen_y_terminal);
                char os[] = "391OS>                                                                                               ";
                for(j = 0; j < 80; j++){
                    //terminal_array[terminal_id].screen_save[j] = *(uint8_t*)(0xB8000 + (j << 1));
                    *(uint8_t*)(0xB8000 + ((j+(terminal_array[1].screen_y_terminal*80)) << 1)) = os[j];
                    *(uint8_t*)(0xB8000 + ((j+(terminal_array[1].screen_y_terminal*80)) << 1) + 1) = 7;
                }
            }
        }

        else if(alt_flag == 1 && scanned_key == 0x3D) // F3
        {
            memcpy((uint8_t*)(terminal_array[terminal_id].keyb_buffer_terminal), (uint8_t*)keyb_buffer, sizeof(keyb_buffer));
            //memcpy((uint32_t*)(terminal_array[terminal_id].vidmem_address_terminal), (uint32_t*)0xB8000, 4096);
            terminal_array[terminal_id].screen_x_terminal = ret_x();
            terminal_array[terminal_id].screen_y_terminal = ret_y();
            int j;
            for(j = 0; j < 2000; j++){
                terminal_array[terminal_id].screen_save[j] = *(uint8_t*)(0xB8000 + (j << 1));
                *(uint8_t*)(0xB8000 + (j << 1)) = terminal_array[2].screen_save[j];
                *(uint8_t*)(0xB8000 + (j << 1) + 1) = 7;
            }

            memset(keyb_buffer, '\0', sizeof(keyb_buffer)); // clears the buffer itself
            clear();
            move_cursor(0, 0);

            for(j = 0; j < 2000; j++){
                //terminal_array[terminal_id].screen_save[j] = *(uint8_t*)(0xB8000 + (j << 1));
                *(uint8_t*)(0xB8000 + (j << 1)) = terminal_array[2].screen_save[j];
                *(uint8_t*)(0xB8000 + (j << 1) + 1) = 7;
            }

            terminal_id = 2;

            memcpy((uint32_t*)keyb_buffer, (uint8_t*)(terminal_array[2].keyb_buffer_terminal), sizeof(keyb_buffer));
            //memcpy((uint32_t*)(0xB8000), (uint32_t*)(terminal_array[2].vidmem_address_terminal), 4096);

            move_cursor(terminal_array[2].screen_x_terminal, terminal_array[2].screen_y_terminal);
            //pcb_t* PCB_ptr = (pcb_t*)(0x800000 - ((id+1)*0x2000));
            if (shell_flag==0) {
                send_eoi(KEYBOARD_IRQ);
                send_eoi(8);
                terminal_array[2].screen_x_terminal = 0;
                for(j = 0; j < 80; j++){
                    //terminal_array[terminal_id].screen_save[j] = *(uint8_t*)(0xB8000 + (j << 1));
                    *(uint8_t*)(0xB8000 + ((j+(terminal_array[2].screen_y_terminal*80)) << 1)) = ' ';
                    *(uint8_t*)(0xB8000 + ((j+(terminal_array[2].screen_y_terminal*80)) << 1) + 1) = 7;
                }
                move_cursor(terminal_array[2].screen_x_terminal, terminal_array[2].screen_y_terminal);
                sys_halt(0);
            }
            else {
                terminal_array[2].screen_x_terminal = 7;
                //terminal_array[2].screen_y_terminal = terminal_array[2].screen_y_terminal+1;
                move_cursor(terminal_array[2].screen_x_terminal, terminal_array[2].screen_y_terminal);
                char os[] = "391OS>                                                                                               ";
                for(j = 0; j < 80; j++){
                    //terminal_array[terminal_id].screen_save[j] = *(uint8_t*)(0xB8000 + (j << 1));
                    *(uint8_t*)(0xB8000 + ((j+(terminal_array[2].screen_y_terminal*80)) << 1)) = os[j];
                    *(uint8_t*)(0xB8000 + ((j+(terminal_array[2].screen_y_terminal*80)) << 1) + 1) = 7;
                }
            }
        }
        break; 
    }

    if(scanned_key == enter)
    {
        enter_flag = 1; // triggers the enter for the read to work
        //keyb_buffer[character_count] = '\n';
        //character_count++;
        //putc('\n');
        //screen_x = ret_x();
        //screen_y = ret_y() + 1;
        //move_cursor(screen_x, screen_y);
        //terminal_write(0, keyb_buffer, character_count);
        //terminal_read(0, keyb_buffer, character_count); // HOW READ AND WRITE TESTING WAS DONE; CALLED HERE TO DISPLAY + ECHO TO TERMINAL
    }

    sti();
    send_eoi(KEYBOARD_IRQ);
    //sti();
}




