
#include "test.h"
#include "stdio.h"
#include "screen.h"
#include "syscall.h"
#include "file.h"

char split_command[10][20] = {0};   //命令  分割用
char path[8][16] = {0};   //当前所在路径
char oldpath[8][16] = {0};   //路径保存用
static void disable_interrupt_shell()
{
    uint32_t cp0_status = get_cp0_status();
    cp0_status &= 0xfffffffe;
    set_cp0_status(cp0_status);
}

static void enable_interrupt_shell()
{
    uint32_t cp0_status = get_cp0_status();
    cp0_status |= 0x01;
    set_cp0_status(cp0_status);
}

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
/*
struct task_info task1 = {(uint32_t)&ready_to_exit_task, USER_PROCESS};
struct task_info task2 = {(uint32_t)&wait_lock_task, USER_PROCESS};
struct task_info task3 = {(uint32_t)&wait_exit_task, USER_PROCESS};*/
struct task_info task1 = {(uint32_t)&test_fs, USER_PROCESS};
struct task_info task2 = {(uint32_t)&rw_task1, USER_PROCESS};
struct task_info task3 = {(uint32_t)&drawing_task1, USER_PROCESS};
/*
struct task_info task4 = {(uint32_t)&semaphore_add_task1, USER_PROCESS};
struct task_info task5 = {(uint32_t)&semaphore_add_task2, USER_PROCESS};
struct task_info task6 = {(uint32_t)&semaphore_add_task3, USER_PROCESS};*/
struct task_info task4 = {(uint32_t)&phy_regs_task1, USER_PROCESS};
struct task_info task5 = {(uint32_t)&phy_regs_task2, USER_PROCESS};
struct task_info task6 = {(uint32_t)&phy_regs_task3, USER_PROCESS};

struct task_info task7 = {(uint32_t)&producer_task, USER_PROCESS};
struct task_info task8 = {(uint32_t)&consumer_task1, USER_PROCESS};
struct task_info task9 = {(uint32_t)&consumer_task2, USER_PROCESS};
struct task_info task10 = {(uint32_t)&barrier_task1, USER_PROCESS};
struct task_info task11 = {(uint32_t)&barrier_task2, USER_PROCESS};
struct task_info task12 = {(uint32_t)&barrier_task3, USER_PROCESS};
struct task_info task13 = {(uint32_t)&SunQuan, USER_PROCESS};
struct task_info task14 = {(uint32_t)&LiuBei, USER_PROCESS};
struct task_info task15 = {(uint32_t)&CaoCao, USER_PROCESS};
struct task_info *test_tasks[16] = {&task1, &task2, &task3,
                                    &task4, &task5, &task6,
                                    &task7, &task8, &task9,
                                    &task10, &task11, &task12,
                                    &task13, &task14, &task15,
                                           };
int num_test_tasks = 3;

char command[20] = {0};   //命令数组
int command_put_index = 0;  //当前命令数组有效下标
int shell_location = 16;    //shell开始位置
int shell_location_new = 16;    //打印位置

int valid_input(char ch){
    if((ch>='a' && ch <= 'z')||(ch>='0' && ch <= '9')|| ch == ' '|| ch == '/' || ch == '.'){
        command[command_put_index] = ch;
        command_put_index++;
    }
    else if(ch == 13){
        //回车命令
        command[command_put_index] = 0;
    }else if(ch == 127){
        //退格删除一个字符   
        command_put_index--; 
        if(command_put_index < 0) command_put_index = 0;  
        command[command_put_index] = 0;     
    }else{  //无有效输入
        return 0;
    }
    return 1;
}
void print_path(char *name){  
	while (*name)
	{
		printf("%c",*(name++));
	}
    printf("/");
}

void shell_print_begin(){
    printf("> root@UCAS_OS:");
    int i;int j;
    printf("/");
    for(i = 0; path[i][0] != 0; ++i){
        print_path(path[i]);
    }
    printf("$");
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
            printk("                                                             ");
    }

    for(i = shell_location + 1; i <= shell_location_new; ++i){
            sys_move_cursor(0, i);
            printf("                                                             ");
    }
    sys_move_cursor(0, shell_location + 1); 
}
void clearall_func(){
    int i = 0;

    for(i = 1; i < shell_location + 1; ++i){
            vt100_move_cursor(0, i);
            printk("                                                             ");
    }
    for(i = 0; i < shell_location; ++i){
            sys_move_cursor(0, i);
            printf("                                                             ");
    }
    clear_func();
}

void exec_func(int test_tasks_num){
    sys_spawn(test_tasks[test_tasks_num]);
    printf("exec process[%d]", test_tasks_num);
}

void run_func(){
    uint32_t address = get_runfile_address();
    printf("run address: %08x    firstc: %08x", address, *(uint32_t *)address);
    task15.entry_point = (uint32_t)address;
    sys_spawn(test_tasks[14]);
    printf("      \n run result:  ");
}

void kill_func(int pid){
    sys_kill(pid);
    printf("kill process pid = %d", pid);
}

void mkfs_func(){
    sys_mkfs();
    /*
    printf("   magic : 0x67373\n");
    printf("   num sector : , start sector : \n");
    printf("   inode map offset : \n");
    printf("   sector map offset : \n");
    printf("   inode offset : \n");
    printf("   data offset : \n");
    printf("   inode entry size : 64B, dir entry size : 32B");
    */
}

void statfs_func(){
    sys_statfs();
}

void ls_func(){
    sys_ls();
}
void mkdir_func(){
    sys_mkdir();
}
void cd_func(){
    sys_cd();
}
void rmdir_func(){
    sys_rmdir();
}
void touch_func(){
    sys_touch();
}
void cat_func(){
    sys_cat();
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
        if(command[total_index] == '/'){
            split_command[cod_index][codc_index] = 0;
            cod_index++;
            codc_index = 0;
            continue;
        }
        split_command[cod_index][codc_index] = command[total_index];
        ++codc_index;
    }
    split_command[cod_index][codc_index] = 0;
    split_command[cod_index+1][0] = 0;

    return cod_index + 1;
}

void do_command(){
    char *ps_cod = "ps";
    char *clear_cod = "clear";
    char *exec_cod = "exec";
    char *kill_cod = "kill";
    char *clearall_cod  = "clearall";
    char *mkfs_cod = "mkfs";
    char *statfs_cod = "statfs";
    char *mkdir_cod = "mkdir";
    char *ls_cod = "ls";
    char *cd_cod = "cd";
    char *rmdir_cod = "rmdir";
    char *touch_cod = "touch";
    char *cat_cod = "cat";
    char *run_cod = ".";
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
    else if(strcmp(command, clearall_cod) == 0){
        //清屏命令
        clearall_func();
    }
    else if(multicod && (strcmp(split_command[0], exec_cod) == 0)){
        //exec命令
        exec_func(str2int(split_command[1]));
    }
    else if(multicod && (strcmp(split_command[0], kill_cod) == 0)){
        //kill命令
        kill_func(str2int(split_command[1]));
    }
    else if(strcmp(command, mkfs_cod) == 0){
        //初始化文件系统
        mkfs_func();
    }
    else if(multicod && (strcmp(split_command[0], mkdir_cod) == 0)){
        //新建目录
        mkdir_func();
    }
    else if(multicod && (strcmp(split_command[0], cd_cod) == 0)){
        //进入目录
        cd_func();
    }
    else if(multicod && (strcmp(split_command[0], rmdir_cod) == 0)){
        //删除目录
        rmdir_func();
    }
    else if(multicod && (strcmp(split_command[0], touch_cod) == 0)){
        //新建文件
        touch_func();
    }
    else if(multicod && (strcmp(split_command[0], cat_cod) == 0)){
        //打印文件
        cat_func();
    }
    else if(strcmp(command, statfs_cod) == 0){
        //打印文件系统信息
        statfs_func();
    }
    else if(strcmp(command, ls_cod) == 0){
        //打印文件信息
        ls_func();
    }
    else if(multicod && (strcmp(split_command[0], run_cod) == 0)){
        //执行可执行文件
        run_func();
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
        disable_interrupt_shell();
        char ch = read_uart_ch();
        enable_interrupt_shell();
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
