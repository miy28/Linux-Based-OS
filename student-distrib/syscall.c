#include "syscall.h"
#include "paging.h"
#include "x86_desc.h"
#include "filesystem.h"

uint32_t processes[6] = {0,0,0,0,0,0};
int32_t parent_id = 0;

/*
file_op_table_init
INPUTS: none
OUTPUTS: none
DESCRIPTION: initialize file operations for all the device drivers
*/
void file_op_table_init()
{
    rtc_d.open = RTC_open;
    rtc_d.close = RTC_close;
    rtc_d.read = RTC_read;
    rtc_d.write = RTC_write;

    stdin_d.open = terminal_open;
    stdin_d.close = terminal_close;
    stdin_d.read = terminal_read;
    stdin_d.write = terminal_write;

    stdout_d.open = terminal_open;
    stdout_d.close = terminal_close;
    stdout_d.read = terminal_read;
    stdout_d.write = terminal_write;

    file_d.open = file_open;
    file_d.close = file_close;
    file_d.read = file_read;
    file_d.write = file_write;

    dir_d.open = directory_open;
    dir_d.close = directory_close;
    dir_d.read = directory_read;
    dir_d.write = directory_write;
}

/*
sys_halt
INPUTS: status
OUTPUTS: none (0)
DESCRIPTION: restores id to the parent process and returns to the current process
*/
int32_t sys_halt(uint8_t status) {
    pcb_t* PCB_ptr = (pcb_t*)(0x800000 - ((id+1)*0x2000)); // Initialize a pointer to a PCB struct based on ID
    processes[id] = 0;
    shell_flag = 1;
    if(PCB_ptr->pid == 0){
        sys_execute((uint8_t*)"shell");
    }

	//restore parent data
    parent_id = ((pcb_t*)(0x800000 - ((PCB_ptr->parent_pid+1)*0x2000)))->parent_pid;
    id = PCB_ptr->parent_pid;
    
    //Restore parent paging
    uint32_t paging_address = 0x800000 + (PCB_ptr->parent_pid * 0x400000);

    page_directory[32].pt_base_address_pde = paging_address>>12;

    asm volatile ("movl %%cr3, %%eax" ::: "%eax", "memory"); // Flush tlb
    asm volatile ("movl %%eax, %%cr3" ::: "%eax", "memory");

    //clear file descriptor array
    int i;
    for (i=0; i<6; i++) {
        PCB_ptr->fd_array[i].flag = 0; //clear FD array by setting flags to 0
    }

    //write parent process
    tss.esp0 = 0x800000 - (((pcb_t*)(0x800000 - ((PCB_ptr->parent_pid+1)*0x2000)))->pid*0x2000) - 4;

    //jumpt to execute return
    asm volatile("andl $0, %eax"); //clear eax
    asm volatile("movb %0, %%al" ::"g"(status):"%eax");
    asm volatile("mov %0, %%esp" ::"r"(PCB_ptr->esp):"%esp");
    asm volatile("mov %0, %%ebp" ::"r"(PCB_ptr->ebp):"%ebp"); //restored variables properly
    asm volatile("jmp EXRET"); //return to correct location in execute

    return 0;
}

/*
sys_execute
INPUTS: command
OUTPUTS: none (0)
DESCRIPTION: parses command file and loads in a new program that the processor will
execute
*/
int32_t sys_execute(const uint8_t* command) {
	uint8_t fname[32];
    int32_t command_length = (int32_t)strlen((int8_t*)command);
    int32_t index = 0;
    int32_t arg_index = 0;

    
    // Paging
    int i = 0;
    while (processes[i]==1) { //skip occupied process slots
        i++;
        if (i==6) { // 6 processes max
            printf("Too many processes");
            return -1;
        }
    }
    processes[i] = 1;
    id = i;

    pcb_t* PCB_ptr = (pcb_t*)(0x800000 - ((id+1)*0x2000)); //start at 8MB, leave 4Kb for each pcb


    //Extracting filename
    while (command[index] != ' ' && command[index] != '\0' && command[index] != '\n') { //these symbols mark end of filename
        fname[index] = command[index];
        index++;
    }
    fname[index] = '\0';


    //extracting args
    while (index<command_length) {
        if (command[index] == ' ') {
            index++;
        }
        else {
            PCB_ptr->arg[arg_index] = command[index];
            arg_index++;
            index++;
        }
    }
    PCB_ptr->arg[arg_index] = '\0';

    //Checking validity
    dentry_t dentry;
    uint8_t header[40];
    if (read_dentry_by_name(fname, &dentry)==-1) { //ensure fname is assosciated with a file
        //return -1;
        fname[0] = 's';
        fname[1] = 'h';
        fname[2] = 'e';
        fname[3] = 'l';
        fname[4] = 'l';
        fname[5] = '\0';
        read_dentry_by_name(fname, &dentry); //reboot shell
    }

    read_data (dentry.inode_idx, 0, header, 40); //read header of file
    if (header[0] != 0x7f || header[1] != 0x45 || header[2] != 0x4c || header[3] != 0x46) { //check magic bits
        fname[0] = 's';
        fname[1] = 'h';
        fname[2] = 'e';
        fname[3] = 'l';
        fname[4] = 'l';
        fname[5] = '\0';
        read_dentry_by_name(fname, &dentry);
        read_data (dentry.inode_idx, 0, header, 40); //reboot shell
    }

    uint32_t paging_address = 0x800000 + (id * 0x400000); //Physical memory address

    page_directory[32].pt_base_address_pde = paging_address>>12; //Align it
    page_directory[32].P = 1; //set corresponding bits
    page_directory[32].RW = 1;
    page_directory[32].US = 1;
    page_directory[32].PS = 1;

    asm volatile ("movl %%cr3, %%eax" ::: "%eax", "memory"); //flush tlb
    asm volatile ("movl %%eax, %%cr3" ::: "%eax", "memory");


    // User program loader
    read_data(dentry.inode_idx, 0, (uint8_t*)0x8048000, ((inode_t*)(filesystem_ptr+1+dentry.inode_idx))->filesize); //load program to correct location in memory

    // PCB

    PCB_ptr->pid = id;
    if (id==0) {//shell
        PCB_ptr->pid = id;
        PCB_ptr->parent_pid = id;
    }
    else {//everything else
        PCB_ptr->pid = id;
        PCB_ptr->parent_pid = parent_id;
        parent_id = id;
    }

    for (i = 0; i<8; i++) {
        if (i==0) {//terminal write
            PCB_ptr->fd_array[i].file_op_table_ptr = &stdin_d;
            PCB_ptr->fd_array[i].inode = 0;
            PCB_ptr->fd_array[i].file_position = 0;
            PCB_ptr->fd_array[i].flag = 1;
        }
        else if (i==1) {//terminal read
            PCB_ptr->fd_array[i].file_op_table_ptr = &stdout_d;
            PCB_ptr->fd_array[i].inode = 0;
            PCB_ptr->fd_array[i].file_position = 0;
            PCB_ptr->fd_array[i].flag = 1;
        }
        else {
            PCB_ptr->fd_array[i].flag = 0;//everything else is flagged as empty
        }
    }


    //Context switch
    uint8_t buf[4] = {header[24], header[25], header[26], header[27]}; //bits 24, 25, 26, 27
    //uint32_t* buf = (uint32_t*) 0x8048000 + 24;
    uint32_t cs_eip;
    uint32_t cs_esp;
    cs_eip = *((uint32_t*)buf);
    //cs_eip = *(buf);
    cs_esp = 0x083ffffc; //bottom of 4mb page, value from slides
    PCB_ptr->esp = cs_esp;

    if (id>0) {//skip if shell
        asm volatile("mov %%esp, %0":"=r"(PCB_ptr->esp)::"%esp"); //save esp and ebp for end of halt
        asm volatile("mov %%ebp, %0":"=r"(PCB_ptr->ebp)::"%ebp");
    }

    tss.esp0 = 0x800000 - (id*0x2000) - 4; //SET tss before priviledge change
    tss.ss0 = KERNEL_DS;

    asm volatile ("pushl %%eax" :: "a"(USER_DS)); //SS
    asm volatile ("pushl %%eax" :: "a"(cs_esp)); //ESP
    asm volatile ("pushfl"); //EFLAGSmake 
    asm volatile ("pushl %%eax" :: "a"(USER_CS)); //Code Segment
    asm volatile ("pushl %%eax" :: "a"(cs_eip)); //Code Segment
    asm volatile ("iret"); //iret
    asm volatile ("EXRET: ");

    return 0;
}
/*
sys_read
INPUTS: int32_t fd (index), void* buf (buffer), int32_t nbytes (passed in number of bytes)
OUTPUTS: none (0)
DESCRIPTION: reads data from the different device drivers for sys call
*/
int32_t sys_read(int32_t fd, void* buf, int32_t nbytes) {
    pcb_t* PCB_ptr = (pcb_t*)(0x800000 - ((id+1)*0x2000));//find pcb location
    if (fd<0 || fd>7) { //check if fd correct
        return -1;
    }
    if (fd==1) {//std check
        return -1;
    }
    if (PCB_ptr->fd_array[fd].flag) {//ensure file present
        return PCB_ptr->fd_array[fd].file_op_table_ptr->read(fd, buf, nbytes);//read according to op table
    }
    return -1;
}
/*
sys_write
INPUTS: int32_t fd (index), void* buf (buffer), int32_t nbytes (passed in number of bytes)
OUTPUTS: none (0)
DESCRIPTION: writes data from the different device drivers for sys call
*/

int32_t sys_write(int32_t fd, const void* buf, int32_t nbytes) {
    pcb_t* PCB_ptr = (pcb_t*)(0x800000 - ((id+1)*0x2000));//find pcb location
    if (fd<0 || fd>7) {//check if fd correct
        return -1;
    }
    if (fd==0) {//std check
        return -1;
    }
    if (PCB_ptr->fd_array[fd].flag) {//ensure file present
        return PCB_ptr->fd_array[fd].file_op_table_ptr->write(fd, buf, nbytes);//write according to op table
    }
    return -1;
}
/*
sys_open
INPUTS: const uint8_t* filename (file that we need to open)
OUTPUTS: none (0)
DESCRIPTION: determines which file to open based on the different dentry file types
*/
int32_t sys_open(const uint8_t* filename) {
    dentry_t dentry;
    pcb_t* PCB_ptr = (pcb_t*)(0x800000 - ((id+1)*0x2000));
    if (strlen((int8_t*)filename)==0 || strlen((int8_t*)filename)>32) {
        return -1;
    }
    if(read_dentry_by_name(filename, &dentry) == -1){ //chcck if filename valid
        return -1;
    }
    int i;
    for(i = 0; i < 8; i++){//loop through fd array
        if((PCB_ptr->fd_array[i].flag) == 1) continue;//check for flag
        PCB_ptr->fd_array[i].inode = dentry.inode_idx;
        PCB_ptr->fd_array[i].file_position = 0;
        PCB_ptr->fd_array[i].flag = 1; // OCCUPIED!
        if(dentry.filetype == 0){ // RTC
            PCB_ptr->fd_array[i].file_op_table_ptr = &rtc_d;//assign based on op table
            RTC_open((uint8_t*)(dentry.filename));
        }
        else if(dentry.filetype == 1){//directory
            PCB_ptr->fd_array[i].file_op_table_ptr = &dir_d;//assign based on op table
            directory_open((uint8_t*)(dentry.filename));
        }
        else if(dentry.filetype == 2){//file
            PCB_ptr->fd_array[i].file_op_table_ptr = &file_d;//assign based on op table
            // printf("yodle\n");
            file_open((uint8_t*)(dentry.filename));
        }
        
        return i;
    }
    return -1;
}
/*
sys_close
INPUTS: const uint32_t* fd
OUTPUTS: -1 if fail
DESCRIPTION: set flag to 0
*/
int32_t sys_close(uint32_t fd) {
    pcb_t* PCB_ptr = (pcb_t*)(0x800000 - ((id+1)*0x2000)); //setup PCB
    if (fd<2 || fd>7) { //check fd
        return -1;
    }
    if (PCB_ptr->fd_array[fd].flag) {
        PCB_ptr->fd_array[fd].flag = 0; //if flag set reset to 0
        return 0;
    }
    return -1;//otherwise fail
}
/*
sys_getargs
INPUTS: buf to load args into, number of bytes to read
OUTPUTS: none
DESCRIPTION: read args from pcb and return them to buf
*/
int32_t sys_getargs(uint8_t* buf, int32_t nbytes) {
    pcb_t* PCB_ptr = (pcb_t*)(0x800000 - ((id+1)*0x2000));//find pcb
    int i;
    if (strlen(PCB_ptr->arg)==0) {
        return -1;
    }
    for (i=0;i<=strlen(PCB_ptr->arg);i++) {
        buf[i] = PCB_ptr->arg[i];//copy contents
    }
    return 0;
}
/*
sys_vidmap
INPUTS: ** screen_start
OUTPUTS: 0
DESCRIPTION: read args from input and return them to buf
*/
int32_t sys_vidmap(uint32_t** screen_start) {
    if ((int)screen_start < 0x8000000) {
        return -1;
    }

    // PD index 32 is 32*4096*1024 = 134217728 = 0x8000000 -> 128 MB
    // let's use some index after 0x8000000 say index 33
    // PD index 33 is 33*4096*1024 = 138412032 = 0x8400000 -> 132 MB (+4MB)
    page_directory[34].P = 1; //PDE setup
    page_directory[34].US = 1;
    page_directory[34].PS = 0;
    page_directory[34].RW = 1;
    page_directory[34].pt_base_address_pde = (uint32_t) page_table_vidmap >> 12; //point to vidmap PTE

    page_table_vidmap[0].P = 1; // new PTE for vidmap
    page_table_vidmap[0].RW = 1;
    page_table_vidmap[0].US = 1;
    page_table_vidmap[0].pt_base_address_pte = 0xB8; // 

    asm volatile ("movl %%cr3, %%eax" ::: "%eax", "memory"); //flush tlb
    asm volatile ("movl %%eax, %%cr3" ::: "%eax", "memory");

    *screen_start = (uint32_t*)(0x8800000); //video memory

    return 0;
}
int32_t sys_set_handler(int32_t signum, void* handler_address) {
    return -1;
}
int32_t sys_sigreturn(void) {
    return -1;
}
