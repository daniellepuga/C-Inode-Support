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

void read_inode(struct inode *in, int inode_num){

}

void write_inode(struct inode *in){
	
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