#include "paging.h"




/*
paging_init
    INPUTS: none
    OUTPUTS: none
    DESCRIPTION: initialize directories and page entries with correct bit mappings
*/
void paging_init() {

    /* PDE */
    int i;
    for(i = 0; i < entries_size; ++i) { // set the unused values empty PDE's
            page_directory[i].P = 0;
            page_directory[i].RW = 0;
            page_directory[i].US = 0;
            page_directory[i].PWT = 0;
            page_directory[i].PCD = 0;
            page_directory[i].A = 0;
            page_directory[i].D = 0;
            page_directory[i].PS = 0;
            page_directory[i].G = 0;
            page_directory[i].avail = 0;

            page_table_vidmap[i].P = 0;
            page_table_vidmap[i].RW = 1;
            page_table_vidmap[i].US = 0;
            page_table_vidmap[i].PWT = 0;
            page_table_vidmap[i].PCD = 1;
            page_table_vidmap[i].A = 0;
            page_table_vidmap[i].D = 0;
            page_table_vidmap[i].PAT = 0;
            page_table_vidmap[i].G = 0;
            page_table_vidmap[i].avail = 0;
            page_table_vidmap[i].pt_base_address_pte = i;
    }

    page_directory[0].P = 1; // set as per page descriptors documentation
    page_directory[0].RW = 1;
    page_directory[0].US = 0;
    page_directory[0].PWT = 0;
    page_directory[0].PCD = 0;
    page_directory[0].A = 0;
    page_directory[0].D = 0;
    page_directory[0].PS = 0;
    page_directory[0].G = 0;
    page_directory[0].avail = 0;
    page_directory[0].pt_base_address_pde = (int) page_table >> 12; //cast to adjust for bit size
    
    page_directory[1].P = 1; // set as per page descriptors documentation
    page_directory[1].RW = 1;
    page_directory[1].US = 0;
    page_directory[1].PWT = 0;
    page_directory[1].PCD = 0;
    page_directory[1].A = 0;
    page_directory[1].D = 0;
    page_directory[1].PS = 1;
    page_directory[1].G = 0;
    page_directory[1].avail = 0;
    page_directory[1].page_base_address = 1; //4 MB


    /* PTE */
    int j;
    for(j = 0; j < entries_size; j++) { // set empty PTE's
        page_table[j].P = 0;
        // may need to init o
    }

    page_table[0xb8].P = 1; // set as per page descriptors documentation
    page_table[0xb8].RW = 1;
    page_table[0xb8].US = 0;
    page_table[0xb8].PWT = 0;
    page_table[0xb8].PCD = 1;
    page_table[0xb8].A = 0;
    page_table[0xb8].D = 0;
    page_table[0xb8].PAT = 0;
    page_table[0xb8].G = 0;
    page_table[0xb8].avail = 0;
    page_table[0xb8].pt_base_address_pte = 0xB8;



/* https://wiki.osdev.org/Setting_Up_Paging */
    asm volatile(
        "movl %0, %%eax             ;"
        "movl %%eax, %%cr3          ;"

        "movl %%cr4, %%eax          ;"
        "orl $0x00000010, %%eax     ;"
        "movl %%eax, %%cr4          ;"

        "movl %%cr0, %%eax          ;"
        "orl $0x80000000, %%eax     ;"
        "movl %%eax, %%cr0          ;"

        :  : "r"(page_directory): "eax" 
    );

}
