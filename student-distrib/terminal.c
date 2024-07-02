#include "terminal.h"
#include "keyboard.h"
#include "i8259.h"
#include "lib.h"
#include "paging.h"


//char user_buf[128];

/*
terminal_open
    INPUTS: filename
    OUTPUTS: 0
    DESCRIPTION: open terminal
*/
int32_t terminal_open(const uint8_t *filename)
{
    //memset(keyb_buffer, '/0', sizeof(keyb_buffer));
    return 0;
}

/*
terminal_close
    INPUTS: fd
    OUTPUTS: 0
    DESCRIPTION: close terminal
*/
int32_t terminal_close(int32_t fd)
{
    return 0;
}

/*
terminal_read
    INPUTS: fd, buf, nbytes
    OUTPUTS: bytes_copied
    DESCRIPTION: read keyboard input
*/
int32_t terminal_read(int32_t fd, void* buf, int32_t nbytes)
{

    int32_t bytes_copied = 0; // tracks the bytes that are copied
    int i;

    if (buf == NULL || nbytes <= 0) 
    {
        // Invalid buffer or request size, cannot proceed.
        return -1;
    }

    sti();

    // Wait for the Enter key to be pressed.
    while(enter_flag != 1) {}

    cli();

    if (nbytes > 128) // for case where nbytes is bigger than 128
    {
        for (i = 0; i < 128; i++)
        {
            ((char*) buf)[i] = keyb_buffer[i]; // copying the keyboard buffer into the passed in terminal buffer
            if (((char*) buf)[i] == '\n' || ((char*) buf)[i] == '\0')
            {
                bytes_copied++;
                break;
            }
        bytes_copied = i + 1;
        //memset(user_buf, '/0', sizeof(user_buf));
        }
        //memset(keyb_buffer, '/0', sizeof(keyb_buffer));
    }
    
    else if (nbytes < 128) // for case that n bytes is less than 128
    {
        for (i = 0; i < character_count && i < 128; i++)
        {
            ((char*) buf)[i] = keyb_buffer[i];
            if (((char*) buf)[i] == '\n' || ((char*) buf)[i] == '\0') // similarly copying data from one buffer to the terminal buffer
            {
                bytes_copied = i + 1;
                //bytes_copied++;
                break;
            }
           
            bytes_copied = i + 1; // Update bytes_copied in case we exit the loop without a newline.
            //bytes_copied++;
        }


        if (i == character_count && ((char*) buf)[i] != '\n') // i - 1?
            {
                ((char*) buf)[i] = '\n';
                bytes_copied = i + 1;
                //bytes_copied++; // Increment bytes_copied since we've added a newline
            }
        
        //memset(keyb_buffer, '/0', sizeof(keyb_buffer));
        //memset(user_buf, '/0', sizeof(user_buf));
    }

        enter_flag = 0;
        character_count = 0; // resets flags and count
        memset(keyb_buffer, '\0', sizeof(keyb_buffer)); // clears the buffer itself

        //memset(keyb_buffer, '/0', sizeof(keyb_buffer));
        //memset(user_buf, '/0', sizeof(user_buf));
        sti();

        return bytes_copied;
}


/*
terminal_write
    INPUTS: fd, buf, nbytes
    OUTPUTS: bytes_copied
    DESCRIPTION: write from keyboard to screen
*/
int32_t terminal_write(int32_t fd, const void* buf, int32_t nbytes)
{
    if (buf == NULL || nbytes <= 0)
    {
        // Invalid buffer or request size, cannot proceed.
        return -1;
    }

    // Cast the void pointer to a char pointer for easy character manipulation
    char *char_buf = (char*)buf;
    int32_t bytes_written = 0;
    int i;

    cli();

    // Write each character from the buffer to the terminal
    for (i = 0; i < nbytes; i++) {
        // Check if the character is not the null terminator before printing
        if (char_buf[i] != '\0')
        {
            putc(char_buf[i]);
        }
        bytes_written = i + 1;
    }

    sti(); // Re-enable interrupts after the critical section
    move_cursor(ret_x(), ret_y());
    return bytes_written; // Return the number of bytes written to the terminal
}


/*
terminal_new_init
    INPUTS: none
    OUTPUTS: none
    DESCRIPTION: initiates the 3 main terminals
*/
void terminal_new_init(){
    // if(terminal_id >= 3 || terminal_id < 0){
    //     return -1;
    // }
    terminal_id = 0;
    int i;
    for(i = 0; i <= 2; i++){
        int32_t terminal_addr = ((0xB8000)>>12) + 1 + i; // start of video mem + 1 + terminal id
        page_table[terminal_addr].P = 1;
        page_table[terminal_addr].RW = 1;
        page_table[terminal_addr].pt_base_address_pte = terminal_addr;

        asm volatile ("movl %%cr3, %%eax" ::: "%eax", "memory"); //flush tlb
        asm volatile ("movl %%eax, %%cr3" ::: "%eax", "memory");

        memcpy((uint8_t*)(terminal_array[i].keyb_buffer_terminal), (uint8_t*)keyb_buffer, sizeof(keyb_buffer));
        terminal_array[i].screen_x_terminal = 0;
        terminal_array[i].screen_y_terminal = 0;
        terminal_array[i].vidmem_address_terminal = 0xB8000 + ((terminal_id + 1)*4096);
        // terminal_array[iy].flag_terminal = 1;

        //write initial text to the terminals
        terminal_array[i].screen_save[0] = 'S';
        terminal_array[i].screen_save[1] = 't';
        terminal_array[i].screen_save[2] = 'a';
        terminal_array[i].screen_save[3] = 'r';
        terminal_array[i].screen_save[4] = 't';
        terminal_array[i].screen_save[5] = 'i';
        terminal_array[i].screen_save[6] = 'n';
        terminal_array[i].screen_save[7] = 'g';
        terminal_array[i].screen_save[8] = ' ';
        terminal_array[i].screen_save[9] = '3';
        terminal_array[i].screen_save[10] = '9';
        terminal_array[i].screen_save[11] = '1';
        terminal_array[i].screen_save[12] = ' ';
        terminal_array[i].screen_save[13] = 'S';
        terminal_array[i].screen_save[14] = 'h';
        terminal_array[i].screen_save[15] = 'e';
        terminal_array[i].screen_save[16] = 'l';
        terminal_array[i].screen_save[17] = 'l';
        // int j;
        // for(j = 0; j < 2000; j++){
        //     *(uint8_t*)(terminal_addr + (j << 1)) = ' ';
        //     *(uint8_t*)(terminal_addr + (j << 1) + 1) = 7;
        // }
    }
    //terminal_array[1].screen_x_terminal = 7;
    terminal_array[1].screen_y_terminal = 1;
    //terminal_array[2].screen_x_terminal = 7;
    terminal_array[2].screen_y_terminal = 1;
    //send_eoi(1); //KEYBOARD_IRQ
    // uint8_t* arg;
    // arg = (uint8_t *)"shell ";
    // asm ("int $0x80" :: "a"(2), "b"(arg));
}
