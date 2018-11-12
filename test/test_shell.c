
#include "test.h"
#include "stdio.h"
#include "screen.h"
#include "syscall.h"

static char read_uart_ch(void)
{
    char ch = 0;
    unsigned char *read_port = (unsigned char *)(0xbfe48000 + 0x00);
    unsigned char *stat_port = (unsigned char *)(0xbfe48000 + 0x05);

    while ((*stat_port & 0x01))
    {
        ch = *read_port;
    }
    return ch;
}

struct task_info task2_4 = {(uint32_t)&lock_task1, USER_THREAD};
struct task_info task2_5 = {(uint32_t)&lock_task2, USER_THREAD};
struct task_info *lock_tasks[16] = {&task2_4, &task2_5};

int num_lock_tasks = 2;

struct task_info task_shell = {(uint32_t)&test_shell, USER_THREAD};

char command[20];   //命令数组
int command_put_index;  //当前命令数组有效下标
int print_location = 16;    //shell开始位置
int print_location_new = 16;    //打印位置

int valid_input(char ch){
    if(ch>='a' && ch <= 'z'){
        command[command_put_index] = ch;
        command_put_index++;
    }
    else if(ch == 13){
        //回车命令
        command[command_put_index] = 0;
    }else if(ch == 8){
        //退格删除一个字符    
        command_put_index--;   
        command[command_put_index] = 0;     
    }else{  //无有效输入
        return 0;
    }
    return 1;
}
void shell_print_begin(){
    printf("> root@UCAS_OS:");
}

void clear_line(){
    sys_move_cursor(1, print_location_new);
    printf("                                                          ");
    sys_move_cursor(1, print_location_new);
}

void do_command(){
    char *ps_cod = "ps";
    //移至新一行运行
    print_location_new++;
    sys_move_cursor(1, print_location_new);

    int screen_cursor_ybefore = screen_cursor_y;
    if(strcmp(command, ps_cod) == 0){
        //列出正运行的进程
        printf("ps_func succesfully done!\n");
        printf("ps_func succesfully done!\n");
    }
    else if(strcmp(command, ps_cod) == 0){
        //清屏命令
    }
    else{  
        //输入错误命令
        printf("invalid command.\n");
    }

    int y_change = screen_cursor_y - screen_cursor_ybefore;
    if(y_change > 0){
        print_location_new += y_change + 1;
        sys_move_cursor(1, print_location_new);
    }else if(y_change = 0){
        print_location_new += 1;
        sys_move_cursor(1, print_location_new);        
    }else {
        print_location_new = print_location;
        sys_move_cursor(1, print_location);         
    }

    //清空命令
    command_put_index = 0;
    command[command_put_index] = 0; 
}

void test_shell()
{
    int i = 0;
    sys_move_cursor(1, print_location);
    sys_move_cursor(1, print_location);
    printf("----------------command---------------\n");
    print_location_new++;
    while (1)
    {
        // read command from UART port
        //disable_interrupt_shell();
        char ch = read_uart_ch();
        //enable_interrupt_shell();
        
        sys_move_cursor(1, print_location_new);
        shell_print_begin();    //显示输入内容   
        printf("%s", command);   
        if(valid_input(ch)){
        // 回车执行命令
            sys_move_cursor(1, print_location_new);
            printf("                                                          ");
            sys_move_cursor(1, print_location_new);
            shell_print_begin();    //显示输入内容   
            printf("%s", command); 
            if(ch == 13)    //回车执行命令
                do_command();    
        }     
    }
}
