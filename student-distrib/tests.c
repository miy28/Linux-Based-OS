#include "tests.h"
#include "x86_desc.h"
#include "lib.h"
#include "terminal.h"
#include "keyboard.h"
#include "rtc.h"
#include "filesystem.h"




#define PASS 1
#define FAIL 0


/* format these macros as you see fit */
#define TEST_HEADER     \
    printf("[TEST %s] Running %s at %s:%d\n", __FUNCTION__, __FUNCTION__, __FILE__, __LINE__)
#define TEST_OUTPUT(name, result)   \
    printf("[TEST %s] Result = %s\n", name, (result) ? "PASS" : "FAIL");


static inline void assertion_failure(){
    /* Use exception #15 for assertions, otherwise
       reserved by Intel */
    asm volatile("int $15");
}




/* Checkpoint 1 tests */


/* IDT Test - Example
 *
 * Asserts that first 10 IDT entries are not NULL
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: Load IDT, IDT definition
 * Files: x86_desc.h/S
 */
int idt_test(){
    TEST_HEADER;


    int i;
    int result = PASS;
    for (i = 0; i < 10; ++i){
        if ((idt[i].offset_15_00 == NULL) &&
            (idt[i].offset_31_16 == NULL)){
            assertion_failure();
            result = FAIL;
        }
    }


    return result;
}




// add more tests here


/* Checkpoint 2 tests */

int terminal_write_test()
{
    int result = PASS;
    char buffer[11] = "abcdefghij";
    buffer[10] = 10;
    int terminal_write_ret = terminal_write(0,(void*)buffer,11);
    if (terminal_write_ret != 11)
    {
        assertion_failure();
        result = FAIL;
    }
    return result;
} 

int terminal_open_test()
{
    TEST_HEADER;
    uint8_t fn;
    int result = PASS;
    if (terminal_open(&fn) == -1)
    {
        result = FAIL;
        assertion_failure();
    }
    return result;
}
int terminal_read_test() {
    printf("Test Terminal:");
    while (1) {

    }
}


int terminal_close_test()
{
    TEST_HEADER;
    int result = PASS;
    int32_t fd = 5;
    if(terminal_close(fd) == -1)
    {
        result = FAIL;
        assertion_failure();
    }
    return result;
}

int test_rtc_open() {
    uint8_t *file;
    RTC_open(file);
      return 1;
}

int test_rtc_rw() {
      int i;
      int fd = 2;
      int rate = 2;
      RTC_write(fd, &rate, 4);
      putc('\n');
      printf("Slow");
    for (i=0;i<10;i++) {
            putc('.');
        RTC_read();
    }
      rate = 16;
      RTC_write(fd, &rate, 4);
      putc('\n');
      printf("Fast");
    for (i=0;i<10;i++) {
            putc('.');
        RTC_read();
    }
      putc('\n');
      printf("Setting frequency to 3, shouldn't change freq");
      rate = 3;
      RTC_write(fd, &rate, 4);
      putc('\n');
      printf("Fast");
    for (i=0;i<10;i++) {
            putc('.');
        RTC_read();
    }
      putc('\n');
      printf("Raising Frequency to 512, dot now every 100 ticks");
      putc('\n');
      printf("Slow");
      rate = 256;
      RTC_write(fd, &rate, 4);
    for (i=0;i<1000;i++) {
            if (i%100==0) {
                  putc('.');
            }
        RTC_read();
    }
      putc('\n');
      printf("Raising Frequency to 1024");
      putc('\n');
      printf("Faster");
      rate = 1024;
      RTC_write(fd, &rate, 4);
    for (i=0;i<1000;i++) {
            if (i%100==0) {
                  putc('.');
            }
        RTC_read();
    }
    putc('\n');
      printf("Raising Frequency to 4096, should behave same as previous line");
      putc('\n');
      printf("Same");
      rate = 4096;
      RTC_write(fd, &rate, 4);
    for (i=0;i<1000;i++) {
            if (i%100==0) {
                  putc('.');
            }
        RTC_read();
    }
    return 1;
}

int test_rtc_close() {
    RTC_close();
    return 1;
}

/*
*/
int32_t fs_test_smallfiles_read(){
	TEST_HEADER;

	// // int32_t fd;
	// // fd = file_open((uint8_t*)"frame0.txt");

	// // printf("fd: %d", fd);

	// uint8_t buffer[1024];

	// // if(fd < 0){
	// // 	puts("Fail");
	// // 	return 0;
	// // }

	// // int32_t bytes_read = file_read(100, buffer);
	// // if(bytes_read < 0){
	// // 	puts("Fail");
	// // 	return 0;
	// // }



   

    // dentry_t dentry;
    // uint8_t* fname = "frame0.txt";
    // read_dentry_by_name(fname, &dentry);
    // int32_t bytes_read = read_data(dentry.inode_idx, 0, buffer, 1024);
	// int i;
	// for(i = 0; i < 1024; i++){
	// 	printf("%c", buffer[i]);
	// }

	// // file_close(fd);
	// // file_close();

	// printf("bytes read: %d", bytes_read);
	// return bytes_read;
    return 0;
}


int32_t fs_test_printfiles(){
	TEST_HEADER;

	int i;
    inode_t* inode_it;
    
	uint32_t filesize;

	dentry_t* dentry_ptr = filesystem_ptr->dentries;
	uint8_t* filename_ptr;

	for(i = 0; i < filesystem_ptr->dentries_stat; i++){
		// inode_it = (inode_t*)(filesystem_addr + 4096 + 4096*(i));
		int inode_offset;
		inode_offset = dentry_ptr[i].inode_idx;
		inode_it = (inode_t*)(filesystem_ptr + 1 + inode_offset);
		filesize = inode_it->filesize;
		filename_ptr = dentry_ptr[i].filename;
		
		// inodes may not b in same order as dentries

		// printf("\nFile: %s		Size: %d", filename_ptr, filesize);
		
		printf("        File: \"%s\" (%d B)\n", filename_ptr, filesize);

	}


	return 1;
}

int32_t fs_test_3(){
	TEST_HEADER;

	dentry_t dentry;
	read_dentry_by_index(4, &dentry);
	uint8_t* a;
	int b; int c;

	a = dentry.filename;
	b = dentry.filetype;
	c = dentry.inode_idx;

	printf("filename: %s || filetype: %d || associated inode #: %d", a, b, c);
	return 1;
}

int32_t sys_test() {
    uint8_t* test;
    test = (uint8_t *)"shell ";
    asm ("int $0x80" :: "a"(2), "b"(test));
    // test = (uint8_t *)"frame0.txt";
    // asm ("int $0x80" :: "a"(5), "b"(test));
    return 1;
}

int32_t launch_shell() {
    uint8_t* test;
    test = (uint8_t *)"shell ";
    asm ("int $0x80" :: "a"(2), "b"(test));
    // test = (uint8_t *)"frame0.txt";
    // asm ("int $0x80" :: "a"(5), "b"(test));
    return 1;
}


/* Checkpoint 3 tests */
/* Checkpoint 4 tests */
/* Checkpoint 5 tests */


/* Test suite entry point */
void launch_tests(){
    // checkpoint 1 tests - Most of the requirements are self explanatorily met by the fact all our code runs, and you can type/open files
    //TEST_OUTPUT("IDT Test", idt_test()); //IDT works
    // checkpoint 2 tests

    //These test RTC, and can all be run at once
    //TEST_OUTPUT("RTC open", test_rtc_open());
    //TEST_OUTPUT("RTC Read/ write", test_rtc_rw());
    //TEST_OUTPUT("RTC Close", test_rtc_close());

    //Terminal can be tested by typing stuff in the shell, and running executables

    //Filesys tests, should be run separately

	//TEST_OUTPUT("filesys_test", fs_test_printfiles());
	//TEST_OUTPUT("filesys_test", fs_test_3());

    //Checkpoint 3 tests - from the shell run testprint, we originally lost a point becuase we pagefaulting when quitting the base shell, now we relaunch the shell

    //Checkpoint 4 tests - all the programs for this week can be run from launch shell. we lost points on fish because our file_read function was broken

    TEST_OUTPUT("Launch Shell", launch_shell());
}



