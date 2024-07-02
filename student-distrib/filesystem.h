#ifndef _FILESYSTEM_H_
#define _FILESYSTEM_H_

#include "types.h"

#define DENTRY_RESERVED_SIZE 24
#define BOOTBLOCK_RESERVED_SIZE 52
#define DENTRIES_AMOUNT 63
#define DATABLOCKS_AMOUNT 1023

#define DENTRY_FILENAME_SIZE 32
#define BLOCK_SIZE 4096

typedef struct dentry_t {
    uint8_t filename[DENTRY_FILENAME_SIZE];
    uint32_t filetype;
    uint32_t inode_idx;
    uint8_t reserved[DENTRY_RESERVED_SIZE];
} dentry_t;

typedef struct bootblock_t {
    uint32_t dentries_stat;
    uint32_t inodes_stat;
    uint32_t datablocks_stat;
    uint8_t reserved[BOOTBLOCK_RESERVED_SIZE];
    dentry_t dentries[DENTRIES_AMOUNT];
} bootblock_t;

typedef struct inode_t {
    uint32_t filesize; // uint32 cuz filesize takes up 4B = 32 bits
    uint32_t datablocks[DATABLOCKS_AMOUNT]; // inode block is 4096B, each datablock # is 4B so there are 4096/4 - filesizeB = 1023
} inode_t;

unsigned int filesystem_addr;
bootblock_t* filesystem_ptr;

void filesystem_init();

int32_t read_dentry_by_name (const uint8_t* fname, dentry_t* dentry);
int32_t read_dentry_by_index (uint32_t index, dentry_t* dentry);
int32_t read_data (uint32_t inode, uint32_t offset, uint8_t* buf, uint32_t length);

int32_t get_filesize(dentry_t* dentry);

int32_t file_open(const uint8_t* fname);
int32_t file_close(int32_t fd);
int32_t file_write(int32_t fd, const void* buf, int32_t nbytes);
int32_t file_read(int32_t fd, void* buf, int32_t nbytes);

int32_t directory_open(const uint8_t* fname);
int32_t directory_close(int32_t fd);
int32_t directory_write(int32_t fd, const void* buf, int32_t nbytes);
int32_t directory_read(int32_t fd, void* buf, int32_t nbytes);


#endif
