#ifndef INODE_H
#define INODE_H

#define FAILED -1
#define BLOCK_SIZE 4096
#define INODE_SIZE 64
#define INODE_FIRST_BLOCK 3

#define INODES_PER_BLOCK (BLOCK_SIZE / INODE_SIZE)

#define INODE_PTR_COUNT 16
#define MAX_SYS_OPEN_FILES 64


struct inode {
    unsigned int size;
    unsigned short owner_id;
    unsigned char permissions;
    unsigned char flags;
    unsigned char link_count;
    unsigned short block_ptr[INODE_PTR_COUNT];

    unsigned int ref_count;  // in-core only
    unsigned int inode_num;
};

int block_num = inode_num / INODES_PER_BLOCK + INODE_FIRST_BLOCK;
int block_offset_bytes = block_offset * INODE_SIZE;
struct inode *find_incore_free(void);
struct inode *find_incore(unsinged int inode_num);
void read_inode(struct inode *in, int inode_num);
void write_inode(struct inode *in);
int flags = read_u8(block + block_offset_bytes + 7);

int ialloc(void);

#endif