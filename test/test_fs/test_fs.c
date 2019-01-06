#include "file.h"
#include "stdio.h"
#include "string.h"
#include "test_fs.h"
#include "test.h"
#include "syscall.h"
#define  RECV_BUFFER_BASE_PHY 0x1f80000
static char buff[64];

/*
void test_fs(void)
{
    int i, j;
    int fd = sys_fopen("1.txt", 2);

    for (i = 0; i < 10; i++)
    {
        sys_fwrite(fd, "hello world!\n", 13);
    }

    for (i = 0; i < 10; i++)
    {
        sys_fread(fd, buff, 13);
        for (j = 0; j < 13; j++)
        {
            printf("%c", buff[j]);
        }
    }

    sys_fclose(fd);
    sys_exit();
}*/
/*
static uint32_t combuff[20];
uint32_t smallbuff[2];
void test_fs(void)
{
    int i, j;
combuff[0] = 0x0c01ee80;
combuff[1] = 0x24040061;
combuff[2] = 0x0c01ee80;
combuff[3] = 0x24040062;
combuff[4] = 0x0c01ee80;
combuff[5] = 0x24040063;
combuff[6] = 0x0c01ee80;
combuff[7] = 0x24040064;
combuff[8] = 0x1108ffff;
combuff[9] = 0x0;
combuff[10] = 0x0;
    int fd = sys_fopen("sfile", 2);

    sys_fwrite(fd, combuff, 40);

    for (i = 0; i < 10; i++)
    {
        sys_fread(fd, smallbuff, 4);

        printf("%08x \n", smallbuff[0]);
    }
    printf("%08x \n", combuff);
    task15.entry_point = (uint32_t)combuff;
    sys_fclose(fd);
    sys_exit();
}
*/

static uint32_t combuff[30];
uint32_t smallbuff[2];


void write_buffer(uint32_t *buffer)
{
    int num_row = 10;    //一一共多少行
    int num_line = 5;   //一共多少列
    int countr, countl; 
    int init_loction = 6;
    int total_num = 0;
    int command_num = 0;
    for(countr = 0; countr < num_row; ++ countr){
        sys_move_cursor(0,init_loction + countr);    

        for(countl = 0; countl < num_line; ++ countl){            
            if(total_num >= 39){
                combuff[command_num] = *(buffer + countr*num_line + countl);
                ++command_num;
                if(command_num > 26)
                    return;
            }
            ++total_num;    
        }
    }
}


void test_fs(void)
{
    int i, j;
    /*
combuff[0] = 0x0c01ee80;
combuff[1] = 0x24040061;
combuff[2] = 0x0c01ee80;
combuff[3] = 0x24040062;
combuff[4] = 0x0c01ee80;
combuff[5] = 0x24040063;
combuff[6] = 0x0c01ee80;
combuff[7] = 0x24040064;
combuff[8] = 0x1108ffff;
combuff[9] = 0x0;
combuff[10] = 0x0;*/
    uint32_t buffer_loc = RECV_BUFFER_BASE_PHY + 0xa0000000;
    buffer_loc += 20 * 1024;
    write_buffer(buffer_loc);
    int fd = sys_fopen("sfile", 2);
    sys_fwrite(fd, combuff, 90);
    sys_move_cursor(1,1);
    for (i = 0; i < 10; i++)
    {
        sys_fread(fd, smallbuff, 4);
        
        printf("%08x \n", smallbuff[0]);
    }
    printf("%08x \n", combuff);
    //task15.entry_point = (uint32_t)combuff;
    sys_fclose(fd);
    sys_exit();
}

