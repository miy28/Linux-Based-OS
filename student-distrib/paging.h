#ifndef _PAGING_H_
#define _PAGING_H_

#include "types.h"
#include "syscall.h"

#define FOUR_K              4096
#define entries_size        1024

typedef struct page_directory_struct{
    union{
        uint8_t val[4];
        struct{ //4KB
            unsigned int P : 1;                                 // present
            unsigned int RW : 1;                                // read/write
            unsigned int US : 1;                                // user/supervisor
            unsigned int PWT : 1;                               // page write-through
            unsigned int PCD : 1;                               // page cache disabled
            unsigned int A : 1;                                 // accessed
            unsigned int D : 1;                                 // dirty
            unsigned int PS : 1;                                // page size
            unsigned int G : 1;                                 // global
            unsigned int avail : 3;                             // available to userspace
            unsigned int pt_base_address_pde : 20;              // page-table base address (PDE 4KB)
        }__attribute((__packed__));
        struct{ //4MB
            unsigned int P : 1;                                 // present
            unsigned int RW : 1;                                // read/write
            unsigned int US : 1;                                // user/supervisor
            unsigned int PWT : 1;                               // page write-through
            unsigned int PCD : 1;                               // page cache disabled
            unsigned int A : 1;                                 // accessed
            unsigned int D : 1;                                 // dirty
            unsigned int PS : 1;                                // page size
            unsigned int G : 1;                                 // global
            unsigned int avail : 3;                             // available to userspace
            unsigned int PAT : 1;                               // page attribute table index
            unsigned int reserved : 9;                          // reserved bits
            unsigned int page_base_address : 10;                // page base address (PDE 4MB)
        }__attribute((__packed__));
    };
}  page_directory_struct;

typedef struct page_table_struct {

    unsigned int P : 1;                                 // present
    unsigned int RW : 1;                                // read/write
    unsigned int US : 1;                                // user/supervisor
    unsigned int PWT : 1;                               // page write-through
    unsigned int PCD : 1;                               // page cache disabled
    unsigned int A : 1;                                 // accessed
    unsigned int D : 1;                                 // dirty
    unsigned int PAT : 1;                               // page size
    unsigned int G : 1;                                 // global
    unsigned int avail : 3;                             // available to userspace
    unsigned int pt_base_address_pte : 20;              // page base address

} __attribute((__packed__)) page_table_struct;

page_table_struct page_table[entries_size] __attribute__((aligned(FOUR_K)));
page_directory_struct page_directory[entries_size] __attribute__((aligned(FOUR_K)));

page_table_struct page_table_vidmap[entries_size] __attribute__((aligned(FOUR_K)));

void paging_init();

#endif
