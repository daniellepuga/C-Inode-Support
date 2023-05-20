#include "block.h"
#include "free.h"
#include "inode.h"


// allocate blocks from theri respective free maps
int ialloc(void){
	unsigned char block[BLOCK_SIZE] = {0};
    // get the inode map
	bread(1, block);
    // locate a free inode
	int num = find_free(block);

    // if there are no free inodes
	if (num == -1) {
		return -1;
	} else {
        // mark it as non free
		set_free(block, num, 0);
        // save inode back out to disk
		bwrite(num, block);
        // then return that number
		return num;
	}
}