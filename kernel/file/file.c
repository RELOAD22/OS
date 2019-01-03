#include "file.h"
#include "screen.h"

#define SECTOR_SIZE 0x200
#define BLOCK_SIZE 0x1000

#define START_BASE_V 0xa0780000
#define INODE_MAP_BASE_V 0xa0780200
#define BLOCK_MAP_BASE_V 0xa0780400
#define INODE_BASE_V 0xa0788400
#define DATA_BASE_V 0xa07d0000

#define START_BASE 0x20000000
#define INODE_MAP_BASE 0x20000200
#define BLOCK_MAP_BASE 0x20000400
#define INODE_BASE 0x20008400
#define DATA_BASE 0x20050000

superblock_t sb;
uint8_t buffer[512];

void sd_card_read(void *dest, uint32_t offset, uint32_t size)
{
    sdread((unsigned char*)dest, offset, size);
}

void sd_card_write(void *dest, uint32_t offset, uint32_t size)
{
    sdwrite((unsigned char*)dest, offset, size);
}

void clear_buffer(uint8_t *buffer)
{
    int i = 0;
    for(i = 0; i < 512; ++i){
        buffer[i] = 0;
    }
}

//n为要改变的第几个数据块，occupied_flag为1表示被占用
void set_block_map(uint32_t n, uint32_t occupied_flag){ 
    uint32_t sector_index = n / 4096;
    uint32_t map_loc_v = BLOCK_MAP_BASE_V + sector_index*SECTOR_SIZE;
 
    set_bit(map_loc_v, n%4096, occupied_flag);
}

void clear_block_map(){
    int init_occu_num = sb.data_offset / 8;
    uint8_t *dest = BLOCK_MAP_BASE_V;
    int i;

    for(i = 0; i < init_occu_num / 8; i++)
        *(dest + i) = 0xff;
        
    for(; i < 64*512; i++)
        *(dest + i) = 0x00;
}


void set_inode_map(uint32_t n, uint32_t occupied_flag){

    set_bit(INODE_MAP_BASE_V, n, occupied_flag);
}

void clear_inode_map(){
    int i;
    uint8_t *dest = INODE_MAP_BASE_V;
    
    for(i = 0; i < 512; i++)
        *(dest + i) = 0x00;
     
}

void set_bit(uint8_t *dest, uint32_t index, uint32_t set_num)
{
    uint32_t index_8 = index / 8;
    uint32_t index_1 = index % 8;
    uint8_t set_8;

    uint8_t offset_index = 7 - index_1;
    dest = dest + index_8;

    if(set_num == 1){
        *dest |= (1<<offset_index);
    }else{
        *dest &= ~ (1<<offset_index);
    }
}

void set_superblock()
{
    superblock_t *ptr_dev_sb = START_BASE_V;

    *ptr_dev_sb =  sb;
}

void init_copy_to_sd()
{
    sd_card_write(START_BASE_V, START_BASE, 640*512);
}

void do_mkfs()
{   /*
    sd_card_read(START_BASE_V, START_BASE, 1*512);
    superblock_t *check_sb = START_BASE_V;
    if(check_sb->magic_number == 0x67373)
    {
        vt100_move_cursor(1,1);
        printk("have init superblock successfully");    
        return;    
    }*/
    sb.magic_number = 0x67373;
    sb.FS_size = 2097152;       //1GB = 2M*512B

    sb.start_sector = 1048576;  //512MB = 1M*512B
    sb.inode_map_offset = 1;    //1
    sb.block_map_offset = 2;    //64
    sb.inode_offset = 66;       //512
    sb.data_offset = 640;       //2M-584
    
    sb.inode_entry_size = 64;   //B
    sb.dir_entry_size = 32;     //B

    vt100_move_cursor(1,1);
    printk("begin init superblock");
    set_superblock();

    vt100_move_cursor(1,2);
    printk("begin init block_map");
    clear_block_map();    //初始化block_map

    vt100_move_cursor(1,3);
    printk("begin init inode_map");
    clear_inode_map();    //初始化inode_map

    vt100_move_cursor(1,4);
    printk("begin copy_to_sd");
    vt100_move_cursor(1,5);
    init_copy_to_sd();

    vt100_move_cursor(20,4);
    printk("--   copy finish");



    init_root_dir();
    vt100_move_cursor(1,5);
    printk("--INIT DIR FINISH");
    //do_statfs();
}

uint32_t check_occupied_inode()
{
    uint8_t *temp;
    uint32_t count = 0;
    int index;
    int i = 0;
    for(i = 0; i < 512; ++i)
    {
        temp = (uint8_t *)(INODE_MAP_BASE_V + i);
        for(index = 0; index < 8; ++index){
            if((*temp & (1<<index)))
                ++count;
        }
    }

    return count;
}
uint32_t check_occupied_block()
{
    uint8_t *temp;
    uint32_t count = 0;
    int index;
    int i = 0;
    for(i = 0; i < 64*512; ++i)
    {
        temp = (uint8_t *)(BLOCK_MAP_BASE_V + i);
        for(index = 0; index < 8; ++index){
            if((*temp & (1<<index)))
                ++count;
        }
    }

    return count;
}

void do_statfs()
{
    vt100_move_cursor(screen_cursor_x, screen_cursor_y+1);
    printk("reading...\n");

    sd_card_read(START_BASE_V, START_BASE, 640*512);
    superblock_t *check_sb = START_BASE_V;
    
    uint32_t occupied_inode = check_occupied_inode();
    uint32_t occupied_block = check_occupied_block();

    printk("magic : 0x%x \n", check_sb->magic_number);
    printk("used block : %d, start sector : %d\n",occupied_block, check_sb->start_sector);
    printk("inode map offset : %d, occupied sector : 1,used : %d\n",check_sb->inode_map_offset, occupied_inode);
    printk("block map offset : %d, occupied sector : 64\n",check_sb->block_map_offset);
    printk("inode offset : %d, occupied sector : \n",check_sb->inode_offset);
    printk("data  offset : %d, occupied block : \n",check_sb->data_offset);
    printk("inode entry size : %dB, dir entry size : %dB",check_sb->inode_entry_size, check_sb->dir_entry_size);
    screen_cursor_y += 7;
}

uint32_t find_free_inode()
{
    uint8_t *temp;
    uint32_t count = 0;
    int index_8;
    uint32_t index = 0;
    int i = 0;
    for(i = 0; i < 512; ++i)
    {
        temp = (uint8_t *)(INODE_MAP_BASE_V + i);
        for(index_8 = 0; index_8 < 8; ++index_8){
            if( (*temp & (1<< (7 - index_8) ) ) == 0 )
                return index;
            ++index;
        }
    }
}
uint32_t find_free_block()
{
    uint8_t *temp;
    uint32_t count = 0;
    int index_8;
    uint32_t index = 0;
    int i = 0;
    for(i = 0; i < 64*512; ++i)
    {
        temp = (uint8_t *)(BLOCK_MAP_BASE_V + i);
        for(index_8 = 0; index_8 < 8; ++index_8){
            if( (*temp & (1<< (7 - index_8) ) ) == 0 )
                return index;
            ++index;
        }
    }
}

inode_t *init_inode(uint32_t inode_num)
{
    inode_t *inode_loc_v = INODE_BASE_V + inode_num * 64;
    inode_loc_v->MODE = 2;
    inode_loc_v->inode_num = inode_num;
    inode_loc_v->Owner_Info = 0;
    inode_loc_v->size = 32;
    set_inode_map(inode_num, 1);
    return inode_loc_v;
}

inode_t *init_file_inode(uint32_t inode_num)
{
    inode_t *inode_loc_v = INODE_BASE_V + inode_num * 64;
    inode_loc_v->MODE = 2;
    inode_loc_v->inode_num = inode_num;
    inode_loc_v->Owner_Info = 0;
    inode_loc_v->size = 0;
    set_inode_map(inode_num, 1);
    return inode_loc_v;
}

const char *current_name=".";
const char *parrent_name="..";

uint32_t init_dentry(inode_t *inode_p, char *newname, uint32_t parrent_inode_num)
{
    uint8_t dentry_buffer[512];
    uint32_t freeblock_num = find_free_block();
    dentry_t *dentry_loc_v = dentry_buffer;

    clear_buffer(dentry_buffer);
    read_block(dentry_loc_v, freeblock_num);

    strcpy(dentry_loc_v->dentryname, newname);
    dentry_loc_v->inode_num = inode_p->inode_num;
    strcpy(dentry_loc_v->dentrycontent[0].name, current_name);
    dentry_loc_v->dentrycontent[0].inode_num = inode_p->inode_num;
    dentry_loc_v->dentrycontent[0].valid = 1;
    strcpy(dentry_loc_v->dentrycontent[1].name, parrent_name);
    dentry_loc_v->dentrycontent[1].inode_num = parrent_inode_num;
    dentry_loc_v->dentrycontent[1].valid = 1;

    int i = 0;
    for(i = 2; i < 16; ++i){
        dentry_loc_v->dentrycontent[i].valid = 0;
    }

    set_block_map(freeblock_num, 1);
    write_block(dentry_loc_v, freeblock_num);

    return freeblock_num;
}

uint32_t init_file()
{
    uint32_t freeblock_num = find_free_block();
    set_block_map(freeblock_num, 1);
    return freeblock_num;
}
const char *bin_name = "bin";
const char *dev_name = "dev";
const char *lib_name = "lib";
const char *etc_name = "etc";
const char *usr_name = "usr";
const char *tmp_name = "tmp";
const char *doc_name = "doc";
const char *include_name = "include";
const char *mydoc_name = "mydoc";

void init_root_dir()
{
    write_map();


    uint32_t root_inode_num = find_free_inode(); 
    read_inode(root_inode_num);
    //初始化根目录的inode
    inode_t *root_inode_p = init_inode(root_inode_num);

    //初始化根目录的dentry并赋值入inode
    root_inode_p->direct_blocks[0] = init_dentry(root_inode_p, current_name, 0);

    //当前所在目录
    inodeofDentry_now = root_inode_num;   
    path[0][0] = 0;

    write_inode(root_inode_num);
    write_map(); 
 
    create_dir(root_inode_num, bin_name); //1
    create_dir(root_inode_num, dev_name); //2
    create_dir(root_inode_num, lib_name); //3
    create_dir(root_inode_num, etc_name); //4
    create_dir(root_inode_num, usr_name); //5
    create_dir(root_inode_num, tmp_name); //6

    create_dir(5, bin_name);    //7
    create_dir(5, doc_name);    //8
    create_dir(5, include_name);    //9
    create_dir(8, mydoc_name);    //10

}

dentry_t *seek_inode_dentry(uint32_t inode_num)
{
    inode_t *inode_p = INODE_BASE_V + inode_num * 64;
    uint32_t block_num = inode_p->direct_blocks[0];
    return START_BASE + block_num * 4096;
}
uint32_t find_in_dentry(dentry_t *dentry, char *findname)
{
    int i = 0; 
    for(i = 0; i < 16; ++i){
        if(dentry->dentrycontent[i].valid == 0)
            continue;
        if(strcmp(dentry->dentrycontent[i].name, findname) == 0)
            return 1;
    }
    return 0;
}
void add_in_dentry(dentry_t *dentry, char *findname, uint32_t inode_num)
{
    int i = 0;
    for(i = 0; i < 16; ++i){
        if(dentry->dentrycontent[i].valid == 0){
            strcpy(dentry->dentrycontent[i].name, findname);
            dentry->dentrycontent[i].inode_num = inode_num;
            dentry->dentrycontent[i].valid = 1;
            return;
        }
    }
}
void create_dir(uint32_t parrent_inode_num, char *c_dentryname)
{
    read_map();
    read_inode(parrent_inode_num);

    //读入父目录inode的目录，判断是否已经存在
    uint8_t parrent_dentry_buffer[512]; 
    clear_buffer(parrent_dentry_buffer);
    dentry_t *parrent_dentry = seek_inode_dentry(parrent_inode_num);
    sd_card_read(parrent_dentry_buffer, parrent_dentry, 1*512);
   
    if(find_in_dentry(parrent_dentry_buffer, c_dentryname) == 1){
        vt100_move_cursor(screen_cursor_x, screen_cursor_y+1);
        printk("create error! same dentry in here");
        screen_cursor_y += 1;
        return;
    }

    /*在父目录不存在*/

    //分配并读入新INODE
    uint32_t inode_num = find_free_inode();   
    read_inode(inode_num);

    //初始化新目录的inode
    inode_t *inode_p = init_inode(inode_num);

    //初始化目录的dentry并赋值入inode
    inode_p->direct_blocks[0] = init_dentry(inode_p, c_dentryname, parrent_inode_num);

    //添加到父目录中
    add_in_dentry(parrent_dentry_buffer, c_dentryname, inode_num);
    sd_card_write(parrent_dentry_buffer, parrent_dentry, 1*512);

    write_inode(inode_num);
    write_map();
}



void delete_dir(uint32_t parrent_inode_num, char *c_dentryname)
{
    read_map();
    read_inode(parrent_inode_num);

    //读入父目录inode的目录，判断是否已经存在
    uint8_t parrent_dentry_buffer[512]; 
    clear_buffer(parrent_dentry_buffer);
    dentry_t *parrent_dentry = seek_inode_dentry(parrent_inode_num);
    sd_card_read(parrent_dentry_buffer, parrent_dentry, 1*512);
   
    if(find_in_dentry(parrent_dentry_buffer, c_dentryname) == 0){
        vt100_move_cursor(screen_cursor_x, screen_cursor_y+1);
        printk("delete error! no such dentry in here");
        screen_cursor_y += 1;
        return;
    }

    dentry_t *parrent_dentry_v = parrent_dentry_buffer;
    /*在父目录存在*/
    int i = 0; 
    for(i = 0; i < 16; ++i){
        if(parrent_dentry_v->dentrycontent[i].valid == 0)
            continue;
        if(strcmp(parrent_dentry_v->dentrycontent[i].name, c_dentryname) == 0)
            break;
    }
    //清除父目录中的记录
    parrent_dentry_v->dentrycontent[i].valid = 0;
    //读入INODE
    uint32_t delete_inode_num = parrent_dentry_v->dentrycontent[i].inode_num;
    read_inode(delete_inode_num);
    inode_t *delete_inode_p = INODE_BASE_V + delete_inode_num * 64;
    //释放map中的空间
    set_inode_map(delete_inode_num, 0);
    set_block_map(delete_inode_p->direct_blocks[0], 0);
    //更改到父目录中
    sd_card_write(parrent_dentry_buffer, parrent_dentry, 1*512);
    write_map();
}

void create_file(uint32_t parrent_inode_num, char *c_filename)
{
    read_map();
    read_inode(parrent_inode_num);

    //读入父目录inode的目录，判断是否已经存在
    uint8_t parrent_dentry_buffer[512]; 
    clear_buffer(parrent_dentry_buffer);
    dentry_t *parrent_dentry = seek_inode_dentry(parrent_inode_num);
    sd_card_read(parrent_dentry_buffer, parrent_dentry, 1*512);
   
    if(find_in_dentry(parrent_dentry_buffer, c_filename) == 1){
        vt100_move_cursor(screen_cursor_x, screen_cursor_y+1);
        printk("create error! same dentry in here");
        screen_cursor_y += 1;
        return;
    }

    /*在父目录不存在*/

    //分配并读入新INODE
    uint32_t inode_num = find_free_inode();   
    read_inode(inode_num);

    //初始化新目录的inode
    inode_t *inode_p = init_file_inode(inode_num);

    //初始化目录的dentry并赋值入inode
    inode_p->direct_blocks[0] = init_file();

    //添加到父目录中
    add_in_dentry(parrent_dentry_buffer, c_filename, inode_num);
    sd_card_write(parrent_dentry_buffer, parrent_dentry, 1*512);

    write_inode(inode_num);
    write_map();
}

void do_touch()
{
    create_file(inodeofDentry_now, split_command[1]);
}

void do_cat()
{
    read_inode(inodeofDentry_now);

    //读入父目录inode的目录，判断是否已经存在
    uint8_t parrent_dentry_buffer[512]; 
    clear_buffer(parrent_dentry_buffer);
    dentry_t *parrent_dentry = seek_inode_dentry(inodeofDentry_now);
    sd_card_read(parrent_dentry_buffer, parrent_dentry, 1*512);
   
    if(find_in_dentry(parrent_dentry_buffer, split_command[1]) == 0){
        vt100_move_cursor(screen_cursor_x, screen_cursor_y+1);
        printk("error! no such file in here");
        screen_cursor_y += 1;
        return;
    }

    dentry_t *parrent_dentry_v = parrent_dentry_buffer;
    /*在父目录存在*/
    int i = 0; 
    for(i = 0; i < 16; ++i){
        if(parrent_dentry_v->dentrycontent[i].valid == 0)
            continue;
        if(strcmp(parrent_dentry_v->dentrycontent[i].name, split_command[1]) == 0)
            break;
    }

    uint32_t file_inode_num = parrent_dentry_v->dentrycontent[i].inode_num;
    read_inode(file_inode_num);
    inode_t *file_inode_p = INODE_BASE_V + file_inode_num * 64;

    uint32_t length = file_inode_p->size;

    uint32_t block_num = file_inode_p->direct_blocks[0];
    uint8_t file_buffer[512]; 
    clear_buffer(file_buffer);
    sd_card_read(file_buffer, START_BASE + block_num * 4096, 1*512);
    vt100_move_cursor(screen_cursor_x, screen_cursor_y+1);
    i = 0;
    while(length--){
        printk("%c", file_buffer[i]);
        ++i;
        if(file_buffer[i] == '\n')
            screen_cursor_y += 1;
    }
}

void do_ls()
{
    read_inode(inodeofDentry_now);
    uint8_t dentry_buffer[512]; 
    clear_buffer(dentry_buffer);

    dentry_t *dentry = seek_inode_dentry(inodeofDentry_now);
    sd_card_read(dentry_buffer, dentry, 1*512);

    dentry_t *dentry_now = dentry_buffer;

    vt100_move_cursor(screen_cursor_x, screen_cursor_y+1);
      
    int i = 0;
    for(i = 0; i < 16; ++i){
        if(dentry_now->dentrycontent[i].valid == 1){
            print_name(dentry_now->dentrycontent[i].name);
            
        }
    }
}

void print_name(char *name){  
	while (*name)
	{
		printk("%c",*(name++));
	}

    printk(" | ");
}

void save_path(){
    int i; int j;
    for(i = 0; i < 8; ++i){
        for(j = 0; j < 16; ++j){
            oldpath[i][j] = path[i][j];
        }
    }
}
void recover_path(){
    int i; int j;
    for(i = 0; i < 8; ++i){
        for(j = 0; j < 16; ++j){
            path[i][j] = oldpath[i][j];
        }
    }
}
void add_path(char *name){
    int i; int j;
    for(i = 0; i < 8; ++i){
        if(path[i][0]==0)
        break;
    }
    if(i == 0){
        if((strcmp(name, current_name) == 0) ||(strcmp(name, parrent_name) == 0) ){
            return;
        }
        strcpy(path[0], name);
        path[1][0] = 0;
    }else{
        if(strcmp(name, current_name) == 0){
            return;
        }
        else if(strcmp(name, parrent_name) == 0){
            path[i - 1][0] = 0;
            return;
        }
        else{
            strcpy(path[i], name);
            path[i + 1][0] = 0;
        }
    }
}
void do_cd(){
    uint32_t beforeinode = inodeofDentry_now;
    save_path();
    char seek_path[5][16] = {0};
    int i = 0;
    int index; int length;
    vt100_move_cursor(1,9);
    print_name(split_command[0]);  
    print_name(split_command[1]);
    print_name(split_command[2]);
    print_name(split_command[3]);
    print_name(split_command[4]);  
    if(split_command[1][0] == 0){
        //绝对路径寻址
        for(i = 2; split_command[i][0] != 0; ++i){
            for(index = 0; split_command[i][index] != 0; ++index){
                seek_path[i - 2][index] = split_command[i][index];
            }
            seek_path[i - 2][index] = 0;
        }

        inodeofDentry_now = 0;  //回到根目录
        path[0][0] = 0;
        length = i - 2;
    }else{
        //相对路径寻址
        for(i = 1; split_command[i][0] != 0; ++i){
            for(index = 0; split_command[i][index] != 0; ++index){
                seek_path[i - 1][index] = split_command[i][index];
            }
            seek_path[i - 1][index] = 0;
        }
        length = i - 1;        
    }
    printk("     ");
    print_name(seek_path[0]);  
    i = 0;
    while(i < length){
        read_inode(inodeofDentry_now);
        uint8_t dentry_buffer[512]; 
        clear_buffer(dentry_buffer);

        dentry_t *dentry = seek_inode_dentry(inodeofDentry_now);
        sd_card_read(dentry_buffer, dentry, 1*512);

        dentry_t *dentry_now = dentry_buffer;

        vt100_move_cursor(1,10);
        //遍历目录
        for(index = 0; index < 16; ++index){
            if(dentry_now->dentrycontent[index].valid == 0)
                continue;
                
            print_name(dentry_now->dentrycontent[index].name);       
            if(strcmp(dentry_now->dentrycontent[index].name, seek_path[i]) == 0){
                //如果找到了相应目录,进入下一次循环
                inodeofDentry_now = dentry_now->dentrycontent[index].inode_num;
                add_path(dentry_now->dentrycontent[index].name);
                break;
            }
        }
        if(index == 16){
            //未找到目录,返回最初的目录
            inodeofDentry_now = beforeinode;
            recover_path();
            vt100_move_cursor(screen_cursor_x, screen_cursor_y+1);
            printk("ERROR,NO SUCH DENTRY!");
            return;
        }
        ++i;
    }
    vt100_move_cursor(1,11);
    printk("inodenow: %d", inodeofDentry_now);

}

void do_mkdir(){
    uint32_t beforeinode = inodeofDentry_now;

    char seek_path[5][16] = {0};
    int i = 0;
    int index; int length;
    vt100_move_cursor(1,9);
    print_name(split_command[0]);  
    print_name(split_command[1]);
    print_name(split_command[2]);
    print_name(split_command[3]);
    print_name(split_command[4]);  
    if(split_command[1][0] == 0){
        //绝对路径寻址
        for(i = 2; split_command[i][0] != 0; ++i){
            for(index = 0; split_command[i][index] != 0; ++index){
                seek_path[i - 2][index] = split_command[i][index];
            }
            seek_path[i - 2][index] = 0;
        }

        inodeofDentry_now = 0;  //回到根目录
        length = i - 2;
    }else{
        //相对路径寻址
        for(i = 1; split_command[i][0] != 0; ++i){
            for(index = 0; split_command[i][index] != 0; ++index){
                seek_path[i - 1][index] = split_command[i][index];
            }
            seek_path[i - 1][index] = 0;
        }
        length = i - 1;        
    }
    printk("     ");
    print_name(seek_path[0]);  
    i = 0;
    while(i < length - 1){
        read_inode(inodeofDentry_now);
        uint8_t dentry_buffer[512]; 
        clear_buffer(dentry_buffer);

        dentry_t *dentry = seek_inode_dentry(inodeofDentry_now);
        sd_card_read(dentry_buffer, dentry, 1*512);

        dentry_t *dentry_now = dentry_buffer;

        vt100_move_cursor(1,10);
        //遍历目录
        for(index = 0; index < 16; ++index){
            if(dentry_now->dentrycontent[index].valid == 0)
                continue;
                
            print_name(dentry_now->dentrycontent[index].name);       
            if(strcmp(dentry_now->dentrycontent[index].name, seek_path[i]) == 0){
                //如果找到了相应目录,进入下一次循环
                inodeofDentry_now = dentry_now->dentrycontent[index].inode_num;
                break;
            }
        }
        if(index == 16){
            //未找到目录,返回最初的目录
            inodeofDentry_now = beforeinode;
            vt100_move_cursor(screen_cursor_x, screen_cursor_y+1);
            printk("ERROR,NO SUCH DENTRY!");
            return;
        }
        ++i;
    }

    create_dir(inodeofDentry_now, seek_path[length - 1]);

    vt100_move_cursor(1,11);
    printk("inode   : %d", inodeofDentry_now);
    inodeofDentry_now = beforeinode;
}

void do_rmdir(){
    uint32_t beforeinode = inodeofDentry_now;

    char seek_path[5][16] = {0};
    int i = 0;
    int index; int length;
    vt100_move_cursor(1,9);
    print_name(split_command[0]);  
    print_name(split_command[1]);
    print_name(split_command[2]);
    print_name(split_command[3]);
    print_name(split_command[4]);  
    if(split_command[1][0] == 0){
        //绝对路径寻址
        for(i = 2; split_command[i][0] != 0; ++i){
            for(index = 0; split_command[i][index] != 0; ++index){
                seek_path[i - 2][index] = split_command[i][index];
            }
            seek_path[i - 2][index] = 0;
        }

        inodeofDentry_now = 0;  //回到根目录
        length = i - 2;
    }else{
        //相对路径寻址
        for(i = 1; split_command[i][0] != 0; ++i){
            for(index = 0; split_command[i][index] != 0; ++index){
                seek_path[i - 1][index] = split_command[i][index];
            }
            seek_path[i - 1][index] = 0;
        }
        length = i - 1;        
    }
    printk("     ");
    print_name(seek_path[0]);  
    i = 0;
    while(i < length - 1){
        read_inode(inodeofDentry_now);
        uint8_t dentry_buffer[512]; 
        clear_buffer(dentry_buffer);

        dentry_t *dentry = seek_inode_dentry(inodeofDentry_now);
        sd_card_read(dentry_buffer, dentry, 1*512);

        dentry_t *dentry_now = dentry_buffer;

        vt100_move_cursor(1,10);
        //遍历目录
        for(index = 0; index < 16; ++index){
            if(dentry_now->dentrycontent[index].valid == 0)
                continue;
                
            print_name(dentry_now->dentrycontent[index].name);       
            if(strcmp(dentry_now->dentrycontent[index].name, seek_path[i]) == 0){
                //如果找到了相应目录,进入下一次循环
                inodeofDentry_now = dentry_now->dentrycontent[index].inode_num;
                break;
            }
        }
        if(index == 16){
            //未找到目录,返回最初的目录
            inodeofDentry_now = beforeinode;
            vt100_move_cursor(screen_cursor_x, screen_cursor_y+1);
            printk("ERROR,NO SUCH DENTRY!");
            return;
        }
        ++i;
    }

    delete_dir(inodeofDentry_now, seek_path[length - 1]);

    vt100_move_cursor(1,11);
    printk("inode   : %d", inodeofDentry_now);
    inodeofDentry_now = beforeinode;
}

int get_inode_from_dennow(char *name){
    read_inode(inodeofDentry_now);

    //读入父目录inode的目录，判断是否已经存在
    uint8_t parrent_dentry_buffer[512]; 
    clear_buffer(parrent_dentry_buffer);
    dentry_t *parrent_dentry = seek_inode_dentry(inodeofDentry_now);
    sd_card_read(parrent_dentry_buffer, parrent_dentry, 1*512);
   
    if(find_in_dentry(parrent_dentry_buffer, name) == 0){
        vt100_move_cursor(screen_cursor_x, screen_cursor_y+1);
        printk("error! no such file in here");
        screen_cursor_y += 1;
        return -1;
    }

    dentry_t *parrent_dentry_v = parrent_dentry_buffer;
    /*在父目录存在*/
    int i = 0; 
    for(i = 0; i < 16; ++i){
        if(parrent_dentry_v->dentrycontent[i].valid == 0)
            continue;
        if(strcmp(parrent_dentry_v->dentrycontent[i].name, name) == 0)
            break;
    }

    return parrent_dentry_v->dentrycontent[i].inode_num;
}


int do_fopen(char *name, int access)
{
    int result = get_inode_from_dennow(name);
    if(result == -1)
        return -1;

    int i = 0;
    for(i = 0; i < 20; ++i){
        if(fd_a[i].valid == 0){
            fd_a[i].inode_num = result;
            fd_a[i].file_wp = 0;
            fd_a[i].file_rp = 0;
            return i;
        }
    }

    fd_a[0].inode_num = result;
    fd_a[0].file_wp = 0;
    fd_a[0].file_rp = 0;
    return 0;
}

int do_fwrite(int fd, char *buff, int size)
{
    uint32_t file_inode_num = fd_a[fd].inode_num;
    read_inode(file_inode_num);
    inode_t *file_inode_p = INODE_BASE_V + file_inode_num * 64;

    uint32_t block_num = file_inode_p->direct_blocks[0];
    uint8_t file_buffer[512]; 
    clear_buffer(file_buffer);
    sd_card_read(file_buffer, START_BASE + block_num * 4096, 1*512);

    int i = 0;

    for(i = 0; i < size; ++i){
		file_buffer[i + fd_a[fd].file_wp] = *(buff++);
	}
    fd_a[fd].file_wp += size;    
    sd_card_write(file_buffer, START_BASE + block_num * 4096, 1*512);

    file_inode_p->size = fd_a[fd].file_wp;
    write_inode(file_inode_num);

    return size;
}


int do_fread(int fd, char *buff, int size)
{
    uint32_t file_inode_num = fd_a[fd].inode_num;
    read_inode(file_inode_num);
    inode_t *file_inode_p = INODE_BASE_V + file_inode_num * 64;

    uint32_t block_num = file_inode_p->direct_blocks[0];
    uint8_t file_buffer[512]; 
    clear_buffer(file_buffer);
    sd_card_read(file_buffer, START_BASE + block_num * 4096, 1*512);

    int i = 0;

    for(i = 0; i < size; ++i){
        *(buff++) = file_buffer[i + fd_a[fd].file_rp];
	}
    fd_a[fd].file_rp += size;   

    return size;
}

void do_fclose(int fd)
{
    fd_a[fd].file_wp = 0;
    fd_a[fd].file_rp = 0;
    fd_a[fd].valid = 0;
    return 0;
}








void read_inode(uint32_t inode_num)
{
    uint32_t inode_sector_v = ((INODE_BASE_V + inode_num * 64)/512) * 512 ;
    uint32_t inode_sector = ((INODE_BASE + inode_num * 64)/512) * 512 ;
    sd_card_read(inode_sector_v, inode_sector, 1*512);    
}

void write_inode(uint32_t inode_num)
{
    uint32_t inode_sector_v = ((INODE_BASE_V + inode_num * 64)/512) * 512 ;
    uint32_t inode_sector = ((INODE_BASE + inode_num * 64)/512) * 512 ;
    sd_card_write(inode_sector_v, inode_sector, 1*512);    
}

void read_block(uint8_t *buffer, uint32_t block_num)
{
    dentry_t *dentry_loc = START_BASE + block_num * 4096;
    sd_card_read(buffer, dentry_loc, 1*512);    
}

void write_block(uint8_t *buffer, uint32_t block_num)
{
    dentry_t *dentry_loc = START_BASE + block_num * 4096;
    sd_card_write(buffer, dentry_loc, 1*512);    
}

void read_map(){
    sd_card_read(INODE_MAP_BASE_V, INODE_MAP_BASE, 1*512);
    sd_card_read(BLOCK_MAP_BASE_V, BLOCK_MAP_BASE, 64*512);    
}
void write_map(){
    sd_card_write(INODE_MAP_BASE_V, INODE_MAP_BASE, 1*512);
    sd_card_write(BLOCK_MAP_BASE_V, BLOCK_MAP_BASE, 64*512);     
}    