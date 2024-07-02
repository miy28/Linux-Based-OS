#include "filesystem.h"
#include "lib.h"
#include "syscall.h"

/*
filesystem_init
    INPUTS: none
    OUTPUTS: none
    DESCRIPTION: Initialize the file system by setting the bootblock address. everything else is based on this
*/
void filesystem_init(){
    filesystem_ptr = (bootblock_t*)filesystem_addr;
}

/*
read_dentry_by_name
    INPUTS: fname, dentry
    OUTPUTS: return 0, -1
    DESCRIPTION: populate given dentry with dentry properties of fname
*/
int32_t read_dentry_by_name (const uint8_t* fname, dentry_t* dentry){
    int i;
    dentry_t* dentries_ptr;
    uint8_t* dentry_fname;
    dentries_ptr = filesystem_ptr->dentries;
    for(i = 0; i < DENTRIES_AMOUNT-1; i++){ // loop thru all dentries
        dentry_fname = dentries_ptr[i].filename;
        if(strncmp((int8_t*)fname, (int8_t*)dentry_fname, DENTRY_FILENAME_SIZE) == 0){ //found matching filename
            strncpy((int8_t*)dentry->filename, (int8_t*)dentry_fname, DENTRY_FILENAME_SIZE); //copy in values
            dentry->filetype = dentries_ptr[i].filetype;
            dentry->inode_idx = dentries_ptr[i].inode_idx;
            if (dentry->filename == (uint8_t*)"shell") {
                shell_flag = 1;
            }
            else {
                shell_flag = 0;
            }
            return 0;
        }
    }
    return -1;
}

/*
read_dentry_by_index
    INPUTS: index, dentry
    OUTPUTS: return 0, -1
    DESCRIPTION: populate given dentry with dentry properties of inode_idx
*/
int32_t read_dentry_by_index (uint32_t index, dentry_t* dentry){
    //int i;
    dentry_t* dentries_ptr;
    //uint8_t* dentry_fname;

    dentries_ptr = filesystem_ptr->dentries;
    // for(i = 0; i < DENTRIES_AMOUNT-1; i++){ // loop thru all dentries
    //     if(dentries_ptr[i].inode_idx == index){ // found matching inode index
    //         dentry_fname = dentries_ptr[i].filename;
    //         strncpy((int8_t*)dentry->filename, (int8_t*)dentry_fname, DENTRY_FILENAME_SIZE); //copy in values
    //         dentry->filetype = dentries_ptr[i].filetype;
    //         dentry->inode_idx = dentries_ptr[i].inode_idx;
    //         return 0;
    //     }
    // }

    strncpy((int8_t*)dentry->filename, (int8_t*)dentries_ptr[index].filename, DENTRY_FILENAME_SIZE); //copy in values
    dentry->filetype = dentries_ptr[index].filetype;
    dentry->inode_idx = dentries_ptr[index].inode_idx;
    return 0;
    
    // return -1;
}

/*
read_data
    INPUTS: 
    OUTPUTS: return copied --- number of bytes copied
    DESCRIPTION: populate given dentry with dentry properties of inode_idx
*/
int32_t read_data (uint32_t inode, uint32_t offset, uint8_t* buf, uint32_t length){
    inode_t* base_inode_ptr;
    inode_t* target_inode_ptr;
    uint32_t* base_datablock_ptr;
    uint32_t* target_datablock_ptr;
    //uint8_t* start_buf = buf;//for debugging

    base_inode_ptr = (inode_t*)(filesystem_ptr + 1) ; //first inode 
    target_inode_ptr = (inode_t*)(filesystem_ptr + 1 + (inode)) ; //inode at inode index inode
    // target_inode_ptr = (inode_t*)(base_inode_ptr + inode-1); //inode param is inode # hence the uint32_t

    base_datablock_ptr = (uint32_t*)(base_inode_ptr + filesystem_ptr->inodes_stat);

    if(inode < 0 || inode > filesystem_ptr->inodes_stat) return -1; //bounds

    if(offset < 0 || offset > ((inode_t*)(filesystem_ptr+1+inode))->filesize) return 0; // stop reading
    if(length > ((inode_t*)(filesystem_ptr+1+inode))->filesize){
        length = ((inode_t*)(filesystem_ptr+1+inode))->filesize;
    }

    int idx_in_datablock;
    int idx_datablock;
    int copied;
    copied=0;
    
    idx_in_datablock = offset % 4096; //remainder gives bleed bytes
    idx_datablock = offset / 4096; //divide and round down gives number of full blocks before bleed bytes

    while(copied < length){ //loop thru datablock starting from offset
        // if(i == (target_inode_ptr->filesize)-1) {
        //     return copied;
        // } // reached limit (end of file)
        if(idx_in_datablock >= 4096){
            idx_datablock = idx_datablock +1;
            idx_in_datablock = 0;
            

        }
        // idx_in_datablock = (offset+i) % 4096; //remainder gives bleed bytes

        // idx_datablock = (offset+i) / 4096; //divide and round down gives number of full blocks before bleed bytes

        target_datablock_ptr = (uint32_t*)((uint8_t*)base_datablock_ptr + (target_inode_ptr->datablocks[idx_datablock])*BLOCK_SIZE + idx_in_datablock); // counts in 1 byte increments
        memcpy(buf, (uint8_t*)target_datablock_ptr, 1); // load in datablocks data into buf one byte at a time
        buf++;
        copied++;
        idx_in_datablock++;
    }
    return copied;
}

/*
get_filesize
    INPUTS: dentry
    OUTPUTS: filesize
    DESCRIPTION: lookup inode->filesize from given dentry
*/
int32_t get_filesize(dentry_t* dentry){
    inode_t* inode_ptr;
    inode_ptr = (inode_t*)(filesystem_ptr + 1 + dentry->inode_idx);
    return inode_ptr->filesize;
}

/*
file_open  
    INPUTS: fname
    OUTPUTS: 0
    DESCRIPTION: open file fname
*/
int32_t file_open(const uint8_t* fname){
    dentry_t dentry;
    read_dentry_by_name(fname, &dentry);
    //printf("Opened \"%s\" (filetype: %d - regular file)\n", (int8_t*)(dentry.filename), dentry.filetype);
    return 0;
}

/*
file_close
    INPUTS: fd
    OUTPUTS: 0
    DESCRIPTION: close file
*/
int32_t file_close(int32_t fd){
    return 0;
}

/*
file_write
    INPUTS: fd, buf, nbytes
    OUTPUTS: -1
    DESCRIPTION: write file
*/
int32_t file_write(int32_t fd, const void* buf, int32_t nbytes){
    return -1;
}

/*
file_read
    INPUTS: fd, buf, nbytes
    OUTPUTS: 0
    DESCRIPTION: read data from inode at fd, get the dentry and load data into buffer
*/
int32_t file_read(int32_t fd, void* buf, int32_t nbytes){
    //dentry_t dentry;
    pcb_t* PCB_ptr = (pcb_t*)(0x800000 - ((id+1)*0x2000));
    

    // read_dentry_by_index(PCB_ptr->fd_array[fd].inode, &dentry);
    // uint8_t* fname = (uint8_t*)(dentry.filename);
    // int count = read_data(dentry.inode_idx, PCB_ptr->fd_array[fd].file_position, (uint8_t*)buf, nbytes);
    int count = read_data(PCB_ptr->fd_array[fd].inode, PCB_ptr->fd_array[fd].file_position, (uint8_t*)buf, nbytes);
    if (count<nbytes) {
        nbytes=count;
    }
    PCB_ptr->fd_array[fd].file_position+=nbytes;
    return count;
}

/*
file_open  
    INPUTS: fname
    OUTPUTS: 0
    DESCRIPTION: open file fname
*/
int32_t directory_open(const uint8_t* fname){
    dentry_t dentry;
    read_dentry_by_name(fname, &dentry);
    //printf("Opened \"%s\" (filetype: %d - dir)\n", (int8_t*)(dentry.filename), dentry.filetype);
    // return get_filesize(&dentry);
    return 0;
}

/*
file_close
    INPUTS: fd
    OUTPUTS: 0
    DESCRIPTION: close file
*/
int32_t directory_close(int32_t fd){
    return 0;
}

/*
directory_write
    INPUTS: fd, buf, nbytes
    OUTPUTS: -1
    DESCRIPTION: write directory
*/
int32_t directory_write(int32_t fd, const void* buf, int32_t nbytes){
    return -1;
}

/*
directory_read
    INPUTS: fd, buf, nbytes
    OUTPUTS: strlen(buf)
    DESCRIPTION: reads filename of directory
*/
int32_t directory_read(int32_t fd, void* buf, int32_t nbytes){
    dentry_t dentry;
    pcb_t* PCB_ptr = (pcb_t*)(0x800000 - ((id+1)*0x2000));//find pcb

    if(read_dentry_by_index(PCB_ptr->fd_array[fd].file_position, &dentry) == -1){//check if valid
        return -1;
    }
    // uint8_t* fname = (uint8_t*)(dentry.filename);
    // char filename[strlen((uint8_t*)(dentry.filename))];

    // void* buf_old = buf;
    char clear[nbytes];
    int i;
    for (i = 0; i<nbytes; i++) {
        clear[i] = '\0';
    }
    memcpy(buf, clear, nbytes);//ckear buf
    memcpy(buf, dentry.filename, nbytes);//copy filename
    //uint8_t* nul = '\0';//null terminate
    //memcpy(buf+32, nul, 1);

    // read_data(dentry.inode_idx, PCB_ptr->fd_array[fd].file_position, buf, nbytes);
    PCB_ptr->fd_array[fd].file_position+=1;//increment in filesystem
    if (strlen((int8_t*)(buf))==0) {
        PCB_ptr->fd_array[fd].file_position=0;
    }
    // buf = buf_old;

    return strlen((int8_t*)(buf));
}
