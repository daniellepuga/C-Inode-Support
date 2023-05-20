#include <string.h>
#include "ctest.h"
#include "image.h"
#include "block.h"
#include "free.h"
#include "inode.h"
#include "mkfs.h"

// macros
#define FREE_BLOCK_MAP_NUM 2
#define BLOCK_SIZE 4096
#define ONLY_ONE 255

void block_for_testing(unsigned char *block, int value) {
	for (int i = 0; i < BLOCK_SIZE; i++) {
		block[i] = value;
	}
}

#ifdef CTEST_ENABLE

void test_bread_and_bwrite(void)
{
	unsigned char test_array[BLOCK_SIZE] = {0};
    
	int block_num = 1;
	image_open("test_image", 0);
	bwrite(block_num, test_array);

	unsigned char read_block[BLOCK_SIZE];

	bread(block_num, read_block);
	CTEST_ASSERT(memcmp(test_array, read_block, BLOCK_SIZE) == 0, "testing reading and writing from the same block");

	image_close();
}

void test_image_open_and_close(void)
{
	int image_fd = image_open("test_image", 0);
	CTEST_ASSERT(image_fd == 3, "testing creating and opening image file");
	CTEST_ASSERT(image_close() == 0, "testing closing image file");

	image_fd = image_open("test_image", 1);
	CTEST_ASSERT(image_fd == 3, "testing creation and truncation of new image");
	CTEST_ASSERT(image_close() == 0, "testing closing file");
}

void test_set_free(void)
{
    // arbitrary value for testing
	int num = 420;

    // computations from project spec
    int byte_num = num / BYTE;
	int bit_num = num % BYTE;

	unsigned char test_array[BLOCK_SIZE] = {0};
	set_free(test_array, num, 1); 
	unsigned char character = test_array[byte_num];
	int bit = (character >> bit_num) & 1;

	CTEST_ASSERT(bit == 1, "testing if in use");

	// set a free bit and get it from character
	set_free(test_array, num, 0); 
	character = test_array[byte_num];
	bit = (character >> bit_num) & 0;	

	CTEST_ASSERT(bit == 0, "testing if free");
}

void test_find_free(void)
{
    // arbitrary value for test
	int num = 69; 
	unsigned char test_block[BLOCK_SIZE];
	block_for_testing(test_block, ONLY_ONE);
	
	// make bit free and check for the bit
	set_free(test_block, num, 0);
	CTEST_ASSERT(find_free(test_block) == num, "testing if find_free locates free block");
}

void test_alloc(void)
{
    // arbitrary value for testing
	int num = 69;
	image_open("test_image", 0);

	unsigned char test_block[BLOCK_SIZE];
	block_for_testing(test_block, ONLY_ONE);
	bwrite(FREE_BLOCK_MAP_NUM, test_block);
	
    // allocate
	int alloc_num = alloc();
	CTEST_ASSERT(alloc_num == -1, "testing with no free blocks");

	block_for_testing(test_block, ONLY_ONE);
	set_free(test_block, num, 0);
	bwrite(FREE_BLOCK_MAP_NUM, test_block);

    // allocate
	alloc_num = alloc();
	CTEST_ASSERT(alloc_num == num, "testing if alloc() finds free block");

	image_close();	
}

void test_ialloc(void)
{
    // arbitrary value for testing
	int num = 21;
	image_fd = image_open("test_image", 0);
	unsigned char test_block[BLOCK_SIZE];

    // test block set to all ones
	block_for_testing(test_block, ONLY_ONE);
	bwrite(1, test_block);

    // allocate
	int ialloc_num = ialloc();
	CTEST_ASSERT(ialloc_num == -1, "testing when no free nodes in inode mp");

    // test block to all ones
	block_for_testing(test_block, ONLY_ONE);
	set_free(test_block, num, 0);

    // write to the map
	bwrite(1, test_block);
	ialloc_num = ialloc();
	CTEST_ASSERT(ialloc_num == num, "testing if icollat finds the free inode");

	image_close();
}

void test_mkfs(void) {

    unsigned char test_block[BLOCK_SIZE] = {0};
    unsigned char compare_block[BLOCK_SIZE] = {127};

    image_open("test_image", 1);
    bwrite(FREE_BLOCK_MAP_NUM, test_block);
    mkfs();
    bread(FREE_BLOCK_MAP_NUM, test_block);
    int free_check = find_free(test_block);

    CTEST_ASSERT((free_check == 7), "testing alloc for mkfs");
    int result = memcmp(test_block, compare_block, BLOCK_SIZE);
    CTEST_ASSERT((result == 0), "testing data for mkfs");
}

int main(void)
{
    CTEST_VERBOSE(1);
	test_image_open_and_close();
	test_bread_and_bwrite();
	test_set_free();
	test_find_free();
	test_alloc();
	test_ialloc();
	test_mkfs();

    CTEST_RESULTS();
    CTEST_COLOR(1);
    CTEST_EXIT();
}

#endif