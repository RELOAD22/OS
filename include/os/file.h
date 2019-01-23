#ifndef INCLUDE_FILE_H_
#define INCLUDE_FILE_H_

#define MAX_NAME_LENGTH 16

#include "type.h"

#include "test.h"

//extern char split_command[10][20];
typedef struct superblock
{
    uint32_t magic_number;
    uint32_t FS_size;

    uint32_t start_sector;
    uint32_t inode_map_offset;    
    uint32_t block_map_offset;
    uint32_t inode_offset;
    uint32_t data_offset;

    uint32_t inode_entry_size;
    uint32_t dir_entry_size;

} superblock_t;

typedef struct filetime
{
    uint32_t create_time; //建立或改变时间
    uint32_t read_time; //最后读取时间
    uint32_t write_time; //最后修改时间
    
} filetime_t;

typedef struct inode
{
    uint8_t MODE;  //0-可读，1-可写，2-读写
    uint8_t inode_num;
    uint8_t Owner_Info;    //进程pid

    uint32_t size;
    //filetime_t timestamps;    

    uint32_t direct_blocks[10];
    uint32_t Indirect_blocks;
    uint32_t Double_Indirect_blocks;
    uint32_t Triple_Indirect_blocks;

} inode_t;

typedef struct dentryenum
{
    char name[MAX_NAME_LENGTH];
    uint32_t inode_num;
    uint8_t valid;
} dentryenum_t;

typedef struct dentry
{
    char dentryname[MAX_NAME_LENGTH];
    uint32_t inode_num;

    dentryenum_t dentrycontent[16];
} dentry_t;

typedef struct fdenum
{
    uint32_t inode_num;
    uint32_t file_wp;
    uint32_t file_rp;
    uint8_t valid;
} fdenum_t;

fdenum_t fd_a[20];

uint8_t runfile_buffer[512]; 

uint32_t inodeofDentry_now;

void sd_card_read(void *dest, uint32_t offset, uint32_t size);

void sd_card_write(void *dest, uint32_t offset, uint32_t size);

void do_mkfs();

void do_statfs();

void do_ls();

void do_mkdir();

void do_cd();

void do_rmdir();

void do_touch();

void do_cat();

int do_fopen(char *name, int access);

int do_fwrite(int fd, char *buff, int size);

int do_fread(int fd, char *buff, int size);

void do_fclose(int fd);

uint32_t get_runfile_address();
#endif