#ifndef _SYSCALL_H_
#define _SYSCALL_H_

#include "terminal.h"
#include "filesystem.h"
#include "rtc.h"
#include "lib.h"
#include "paging.h"
int32_t id;
int32_t shell_flag;
// file operation table
typedef struct {
    int32_t (*open)(const uint8_t* fname);
    int32_t (*close)(int32_t fd);
    int32_t (*read)(int32_t fd, void* buf, int32_t nbytes);
    int32_t (*write)(int32_t fd, const void* buf, int32_t nbytes);
} file_op_table;

file_op_table rtc_d;
file_op_table stdin_d;
file_op_table stdout_d;
file_op_table file_d;
file_op_table dir_d;

// file descriptor table
typedef struct {
    file_op_table* file_op_table_ptr;
    uint32_t inode;
    uint32_t file_position;
    uint32_t flag;
} file_desc_table;

/* pcb */
typedef struct{
    file_desc_table fd_array[8];
    int8_t arg[180];
    uint32_t pid; // current process id
    uint32_t parent_pid; // parent process id
    uint32_t esp; // stack pointer
    uint32_t ebp; // base pointer
    uint32_t eip; // instruction pointer ?

    // keeping it as this for now, maybe add and subtract stuff later
} pcb_t;

int32_t sys_halt(uint8_t status);
int32_t sys_execute(const uint8_t* command);
int32_t sys_read(int32_t fd, void* buf, int32_t nbytes); 
int32_t sys_write(int32_t fd, const void* buf, int32_t nbytes); 
int32_t sys_open(const uint8_t* filename);
int32_t sys_close(uint32_t fd);
int32_t sys_getargs(uint8_t* buf, int32_t nbytes);
int32_t sys_vidmap(uint32_t** screen_start);
int32_t sys_set_handler(int32_t signum, void* handler_address);
int32_t sys_sigreturn(void);

void file_op_table_init();


#endif
