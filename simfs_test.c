#include <string.h>
#include "ctest.h"
#include "image.h"
#include "block.h"
#include "free.h"
#include "inode.h"
#include "mkfs.h"
#include "pack.h"

// macros
#define FREE_BLOCK_MAP_NUM 2
#define BLOCK_SIZE 4096
#define ONLY_ONE 255
#define NEW_INODE_NUM 256

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
	struct inode *ialloc_inode = ialloc();
	CTEST_ASSERT(ialloc_inode == NULL, "testing when no free nodes in inode mp");

    // test block to all ones
	block_for_testing(test_block, ONLY_ONE);
	set_free(test_block, num, 0);

    // write to the map
	bwrite(1, test_block);
	ialloc_inode = ialloc();
	int ialloc_num = ialloc_inode->inode_num;
	CTEST_ASSERT(ialloc_num == num, "testing if ialloc finds the free inode");

	mark_incore_in_use();
	ialloc_inode = ialloc();
	CTEST_ASSERT(ialloc_inode == NULL, "testing no freeincore inodes");

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

void test_find_incore(void)
{
	unsigned int test_num = 34;
	clear_incore_inodes();
	struct inode *free_inode = find_incore_free();
	free_inode->inode_num = test_num;
	free_inode->ref_count = 1;
	struct inode *found_incore = find_incore(test_num);
	CTEST_ASSERT(memcmp(free_inode, found_incore, INODE_SIZE) == 0, "testing finding free inode and find by inode number");

	mark_incore_in_use();
	free_inode = find_incore_free();
	CTEST_ASSERT(free_inode == NULL, "testing if no free incore inodes");
	clear_incore_inodes();
	free_inode = find_incore(test_num);
	CTEST_ASSERT(free_inode == NULL, "testing if no incore inode fond with inode number");	
}

void test_read_and_write_inode(void)
{
	unsigned int test_num = 420;
	image_open("test_image", 0);
	struct inode *new_inode = find_incore_free();

	// arbitrary values for testing
	new_inode->inode_num = test_num;
	new_inode->size = 420;
	new_inode->owner_id = 5;
	new_inode->permissions = 10;
	new_inode->flags = 15;
	new_inode->link_count = 20;
	new_inode->block_ptr[0] = 25;
	write_inode(new_inode);
	struct inode inode_read_buffer = {0};
	read_inode(&inode_read_buffer, test_num);
	image_close();

	// test that all values match
	CTEST_ASSERT(new_inode->size == inode_read_buffer.size, "testing if write/read size are the same");
	CTEST_ASSERT(new_inode->owner_id == inode_read_buffer.owner_id, "testing for write/read owner_id are matching");
	CTEST_ASSERT(new_inode->permissions == inode_read_buffer.permissions, "testing to see write/read permissions are matching");
	CTEST_ASSERT(new_inode->flags == inode_read_buffer.flags, "testing if write/read flags are matching");
	CTEST_ASSERT(new_inode->link_count == inode_read_buffer.link_count, "testing if write/read link_count are matching");
	CTEST_ASSERT(new_inode->block_ptr[0] == inode_read_buffer.block_ptr[0], "testing if write/read block_ptr[0] are matching");
}

void test_iget(void)
{
	image_open("test_image", 0);
	unsigned int test_num = 69;
	struct inode *available_inode = find_incore_free();

	CTEST_ASSERT(available_inode->ref_count == 0, "testing if ref_count is initially zero");

	// give arbitrary value to the inode
	available_inode->inode_num = test_num;
	struct inode *iget_inode = iget(test_num);

	CTEST_ASSERT(iget_inode->ref_count == 1, "testing if ref_count +1 after iget");
	CTEST_ASSERT(memcmp(available_inode, iget_inode, INODE_SIZE) == 0, "testing if the right inode is gotten");

	clear_incore_inodes();
	iget_inode = iget(test_num);

	CTEST_ASSERT(iget_inode != NULL, "testing if a new incore is returned");
	CTEST_ASSERT(iget_inode->inode_num == test_num, "testing if new inode has right inode_num");

	unsigned int new_inode_num = NEW_INODE_NUM;
	mark_incore_in_use();
	iget_inode = iget(new_inode_num);

	CTEST_ASSERT(iget_inode == NULL, "testing case when no free incore inode and inod number nonexistant");
	clear_incore_inodes();
	image_close();
}

void test_iput(void)
{
	image_open("test_image", 0);
	// arbitrary value for testing
	unsigned int test_num = 50;
	struct inode *fake_inode = iget(test_num);

	iput(fake_inode);
	CTEST_ASSERT(fake_inode->ref_count == 0, "testing if ref_count -1 after iput");

	struct inode *iget_inode = iget(test_num);

	CTEST_ASSERT(memcmp(fake_inode, iget_inode, INODE_SIZE) == 0, "testing iput and iget togehter");
	image_close();
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
	test_find_incore();
	test_read_and_write_inode();
	test_iget();
	test_iput();

    CTEST_RESULTS();
    CTEST_COLOR(1);
    CTEST_EXIT();
}

#endif