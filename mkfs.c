#include "block.h"
#include "mkfs.h"
#include "image.h"
#include <unistd.h>

// construct the file system
// 1. zero out every block of the file system.
// 2. mark blocks 0-6 as allocated in the free block map.
// 3. (not for this project) add the root directory and other 
//    things to bootstrap the file system

// create the file system
void mkfs(void){
    unsigned char zero_block[BLOCK_SIZE];
    // write 1024 blocks of all zero bytes
    for(int i = 0; i < NUM_BLOCKS; i++){
        write(image_fd, zero_block, BLOCK_SIZE);
    }
    // mark data blocks 0-6 as allocated with alloc()
    // 7 times
    for(int i = 0; i < 7; i++){
        alloc();
    }
    return;
}