#include "block.h"
#include "free.h"
#include "inode.h"
#include "pack.h"
#include <string.h>
#include <stdio.h>

static struct inode incore[MAX_SYS_OPEN_FILES] = {0};

// find the first free in-core inode within the incore array
struct inode *find_incore_free(void){
	for (int i = 0; i < MAX_SYS_OPEN_FILES; i++) {
		// check if reference is 0
		struct inode *temp_incore = &incore[i];

		if (temp_incore->ref_count == 0) {
			return temp_incore;
		}
	}
	return NULL;
}

// find an incore inode record in the incore array
// by the inode number
struct inode *find_incore(unsigned int inode_num){
	for (int i = 0; i < MAX_SYS_OPEN_FILES; i++) {
		// check if reference is 0
		struct inode *temp_incore = &incore[i];

		if(temp_incore->inode_num == inode_num){
			if(temp_incore->ref_count != 0){
				return temp_incore;
			}
		}
	}
	return NULL;
}

// take a pointer to an empty struct inode to read
// data into.
void read_inode(struct inode *in, int inode_num){
	// helper code from project spec
	int block_num = inode_num / INODES_PER_BLOCK + INODE_FIRST_BLOCK;
	int block_offset = inode_num % INODES_PER_BLOCK;
	int block_offset_bytes = block_offset * INODE_SIZE;
	unsigned char read_buffer[BLOCK_SIZE];
	bread(block_num, read_buffer);

	// layout of data as stored on disk for each record
	// read using functions from pack.c
    in->size = read_u32(read_buffer + block_offset_bytes);
    in->owner_id = read_u16(read_buffer + block_offset_bytes + OWNER_ID_OFFSET);
    in->permissions = read_u8(read_buffer + block_offset_bytes + PERMISSIONS_OFFSET);
    in->flags = read_u8(read_buffer + block_offset_bytes + FLAGS_OFFSET);
    in->link_count = read_u8(read_buffer + block_offset_bytes + LINK_COUNT_OFFSET);

    // read pointers to inode using functions from pack.c
    int block_pointer_address = BLOCK_POINTER_OFFSET;
    for (int i = 0; i < INODE_PTR_COUNT; i++) {
    	in->block_ptr[i] = read_u16(read_buffer + block_offset_bytes + block_pointer_address);
    	block_pointer_address += 2;
    }	
}

// stores inode data pointed to by in on disk
void write_inode(struct inode *in){
	int inode_num = in->inode_num;
	// helper code from project spec
	int block_num = inode_num / INODES_PER_BLOCK + INODE_FIRST_BLOCK;
	int block_offset = inode_num % INODES_PER_BLOCK;
	int block_offset_bytes = block_offset * INODE_SIZE;
	unsigned char write_buffer[BLOCK_SIZE];

	// layout of data as stored on disk for each record
	// write functions from pack.c
 	write_u32(write_buffer + block_offset_bytes, in->size);
    write_u16(write_buffer + block_offset_bytes + OWNER_ID_OFFSET, in->owner_id);
    write_u8(write_buffer + block_offset_bytes + PERMISSIONS_OFFSET, in->permissions);
    write_u8(write_buffer + block_offset_bytes + FLAGS_OFFSET, in->flags);
    write_u8(write_buffer + block_offset_bytes + LINK_COUNT_OFFSET, in->link_count);
    // write pointers to inode using functions from pack.c
    int block_pointer_address = BLOCK_POINTER_OFFSET;
    for (int i = 0; i < INODE_PTR_COUNT; i++) {
    	write_u16(write_buffer + block_offset_bytes + block_pointer_address, in->block_ptr[i]);
    	block_pointer_address += 2;
    }
    // write to disk
	bwrite(block_num, write_buffer);
}



// allocate blocks from theri respective free maps
int ialloc(void){
	unsigned char block[BLOCK_SIZE] = {0};
    // get the inode map
	bread(1, block);
    // locate a free inode
	int num = find_free(block);

    // if there are no free inodes
	if (num == FAILED) {
		return FAILED;
	} else {
        // mark it as non free
		set_free(block, num, 0);
        // save inode back out to disk
		bwrite(num, block);
        // then return that number
		return num;
	}
}