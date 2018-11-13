
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
//shell进程
struct task_info task_shell = {(uint32_t)&test_shell, USER_THREAD};
//testlock进程
struct task_info task2_4 = {(uint32_t)&lock_task1, USER_THREAD};
struct task_info task2_5 = {(uint32_t)&lock_task2, USER_THREAD};
struct task_info *lock_tasks[16] = {&task2_4, &task2_5};
int num_lock_tasks = 2;
//test进程
struct task_info task1 = {(uint32_t)&ready_to_exit_task, USER_PROCESS};
struct task_info task2 = {(uint32_t)&wait_lock_task, USER_PROCESS};
struct task_info task3 = {(uint32_t)&wait_exit_task, USER_PROCESS};
struct task_info *test_tasks[16] = {&task1, &task2, &task3,
                                           };
int num_test_tasks = 3;

char command[20] = {0};   //命令数组
int command_put_index = 0;  //当前命令数组有效下标
int shell_location = 16;    //shell开始位置
int shell_location_new = 16;    //打印位置

int valid_input(char ch){
    if((ch>='a' && ch <= 'z')||(ch>='0' && ch <= '9')|| ch == ' '){
        command[command_put_index] = ch;
        command_put_index++;
    }
    else if(ch == 13){
        //回车命令
        command[command_put_index] = 0;
    }else if(ch == 127){
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
    sys_move_cursor(0, shell_location_new);
    printf("                                                          ");
    sys_move_cursor(0, shell_location_new);
}

void clear_func(){
    int i = 0;

    for(i = shell_location + 2; i <= shell_location_new + 1; ++i){
            vt100_move_cursor(0, i);
            printk("                                                          ");
    }

    for(i = shell_location + 1; i <= shell_location_new; ++i){
            sys_move_cursor(0, i);
            printf("                                                             ");
    }
    sys_move_cursor(0, shell_location + 1); 
}

void exec_func(int test_tasks_num){
    sys_spawn(test_tasks[test_tasks_num]);
    printf("exec process[%d]", test_tasks_num);
}

int str2int(const char *str)
{
    int temp = 0;
    const char *ptr = str;  //ptr保存str字符串开头

    if (*str == '-' || *str == '+')  //如果第一个字符是正负号，则移到下一个字符
        str++;

    while(*str != 0)
    {
        if ((*str < '0') || (*str > '9'))  //如果当前字符不是数                        //则退出循环
            break;
        temp = temp * 10 + (*str - '0'); //如果当前字符是数字则计算数值
        str++;      //移到下一个字符
    }   
    if (*ptr == '-')     //如果字符串是以“-”开头，则转换成其相反数
        temp = -temp;

    return temp;
}

char split_command[3][20] = {0};   //命令  分割用

int command_split(const char *str){
    //分割命令,返回命令的个数
    int total_index;
    int cod_index = 0;
    int codc_index = 0;
    for(total_index = 0; total_index < command_put_index; ++total_index){
        if(command[total_index] == ' '){
            split_command[cod_index][codc_index] = 0;
            cod_index++;
            codc_index = 0;
            continue;
        }
        split_command[cod_index][codc_index] = command[total_index];
        ++codc_index;
    }
    split_command[cod_index][codc_index] = 0;

    return cod_index + 1;
}

void do_command(){
    char *ps_cod = "ps";
    char *clear_cod = "clear";
    char *exec_cod = "exec";
    int multicod = 0;

    if(command_split(command) > 1){
        multicod = 1;
    }
    //移至新一行运行
    shell_location_new++;
    sys_move_cursor(0, shell_location_new);

    int screen_cursor_ybefore = screen_cursor_y;
    if(strcmp(command, ps_cod) == 0){
        //列出正运行的进程
        sys_ps();
    }
    else if(strcmp(command, clear_cod) == 0){
        //清屏命令
        clear_func();
    }
    else if(multicod && (strcmp(split_command[0], exec_cod) == 0)){
        //exec命令
        exec_func(str2int(split_command[1]));
    }
    else{  
        //输入错误命令
        printf("invalid command.please try again.");
    }

    int y_change = screen_cursor_y - screen_cursor_ybefore;
    if(y_change > 0){
        shell_location_new += y_change + 1;
        sys_move_cursor(0, shell_location_new);
    }else if(y_change == 0){
        shell_location_new += 1;
        sys_move_cursor(0, shell_location_new);        
    }else {
        shell_location_new = shell_location + 1;
        sys_move_cursor(0, shell_location + 1);         
    }

    //清空命令
    int i;
    command_put_index = 0;
    for(i = 0; i < 20; ++i){
        command[i] = 0;         
    }
}

void test_shell()
{
    int i = 0;
    command_put_index = 0;
    for(i = 0; i < 20; ++i){
        command[i] = 0;         
    }
    shell_location = 16;
    shell_location_new = 16;
    sys_move_cursor(0, shell_location);
    sys_move_cursor(0, shell_location);
    printf("----------------command---------------\n");
    shell_location_new++;
    while (1)
    {
        // read command from UART port
        //disable_interrupt_shell();
        char ch = read_uart_ch();
        //enable_interrupt_shell();
        sys_move_cursor(0, shell_location_new);
        shell_print_begin();    //显示输入内容   
        printf("%s", command); 

        if(valid_input(ch)){
        // 回车执行命令
            sys_move_cursor(0, shell_location_new);
            printf("                                                          ");
            sys_move_cursor(0, shell_location_new);
            shell_print_begin();    //显示输入内容   
            printf("%s", command); 
            if(ch == 13)    //回车执行命令
                do_command();    
            sys_move_cursor(0, shell_location_new);
            printf("                                                          ");
            sys_move_cursor(0, shell_location_new);
        }     
    }
}
