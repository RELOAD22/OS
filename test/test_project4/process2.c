#include "sched.h"
#include "stdio.h"
#include "syscall.h"
#include "time.h"
#include "screen.h"
#include "test4.h"
#define  RW_TIMES 2

int rand()
{	
	int current_time = get_timer();
	return current_time % 100000;
}

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

int valid_input_p(char ch){
    if(ch>='a' && ch <= 'f'){
        return ch - 'a' + 10;	//10-15
    }
	else if(ch>='0' && ch <= '9'){
        return ch - '0';     //0-9
    }
    else if(ch == 13){
        //回车命令
        return 16;
    }else{  //无有效输入
        return -1;
    }
}

static void scanf(int *mem)
{
	//TODO:Use read_uart_ch() to complete scanf(), read input as a hex number.
	//Extending function parameters to (const char *fmt, ...) as printf is recommended but not required.
	char ch;
	char input_flag;
	int Result = 0;
	disable_interrupt_shell();
	vt100_move_cursor(1, 6);
	printk("input:      ");	
	while(1){
		ch = read_uart_ch();
		input_flag = valid_input_p(ch);
		if (input_flag == 16)
			break;
		if(input_flag == -1)
			continue;
		Result *= 16;
		Result += input_flag;
		vt100_move_cursor(1, 6);
		printk("input:%x      ", Result);		
	}
	*mem = Result;
    enable_interrupt_shell();
}

void rw_task1(void)
{
	int mem1, mem2 = 0;
	int curs = 1;
	int memory[4];
	int i = 0;
	int count;
	for(i = 0; i < RW_TIMES; i++)
	{
		vt100_move_cursor(1, curs+i);
		scanf(&mem1);
		vt100_move_cursor(1, curs+i);
		memory[i] = mem2 = rand();
		*(int *)mem1 = mem2;
		vt100_move_cursor(1, curs+i);
		vt100_move_cursor(1, curs+i);
		printk("Write: 0x%x, %d", mem1, mem2);
		/*
		vt100_move_cursor(1, 3);
		printk("err");
		count = *(int *)mem1;
		printk("%x",count);
		printk("success");*/
	}
	curs = 1 + RW_TIMES;
	for(i = 0; i < RW_TIMES; i++)
	{
		vt100_move_cursor(1, curs+i);
		scanf(&mem1);
		vt100_move_cursor(1, curs+i);
		memory[i+RW_TIMES] = *(int *)mem1;
		if(memory[i+RW_TIMES] == memory[i])
			printk("Read succeed: %d", memory[i+RW_TIMES]);
		else
			printk("Read error: %d", memory[i+RW_TIMES]);
	}
	while(1);
	//Only input address.
	//Achieving input r/w command is recommended but not required.
}
