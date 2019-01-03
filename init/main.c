/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * * * * * *
 *            Copyright (C) 2018 Institute of Computing Technology, CAS
 *               Author : Han Shukai (email : hanshukai@ict.ac.cn)
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * * * * * *
 *         The kernel's entry, where most of the initialization work is done.
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * * * * * *
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this 
 * software and associated documentation files (the "Software"), to deal in the Software 
 * without restriction, including without limitation the rights to use, copy, modify, 
 * merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit 
 * persons to whom the Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE. 
 * 
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * * * * * */

#include "irq.h"
#include "test.h"
#include "stdio.h"
#include "sched.h"
#include "screen.h"
#include "mac.h"
#include "common.h"
#include "syscall.h"
#include "time.h"
#include "shell.h"
#include "file.h"
//注意在shell.c中也有该宏定义
#define STACK_BASE  0xa0f00000
#define STACK_SIZE 0x100000

/* ready queue to run */
queue_t ready_queue;

/* block queue to wait */
queue_t block_queue;

/* recv wait queue to wait */
queue_t recv_wait_queue;

/* current running task PCB */
pcb_t *current_running;
pid_t process_id;
pcb_t pcb[NUM_MAX_TASK];
page_t page[NUM_MAX_TASK][256];
int stack_temp;
int tlb_unused_index;
int physical_unused_num;
int disk_unused_num;
int pyhsical_page_full_flag;	//0-not full  1-full

extern uint32_t time_elapsed;	//time.c

static void init_page_table()
{
	int i = 0;
	int num_id;
	for(num_id = 1; num_id < NUM_MAX_TASK; ++num_id){
		for(i = 0; i < 256; ++i){
			//page[num_id][i].virtual_pageframe_num = i;
			//page[num_id][i].physical_pageframe_num = 0x1000 + i + (num_id-1) * 0x100;
			page[num_id][i].valid_flag = 0;
			page[num_id][i].on_disk_flag = 0;
		}
	}
}

static void init_TLB(){

	int k1;
	int vpn2;
	int asid;
	int epfn;
	int coherency;
	int opfn;
	int Dirty;
	int Valid;
	int Global;
	int index_of_some_entry;
	int i;
	for(i = 0; i < 64; i += 2)
	{
		vpn2 = page[3][i].virtual_pageframe_num >> 1;
		asid = 0;
		k1 = (vpn2<<13)|(asid & 0xff);
		set_C0_ENHI(k1);

		coherency = 2; Dirty = 1; Valid = 1; Global = 1;


		epfn = page[3][i].physical_pageframe_num;
		k1 = (epfn<<6)|(coherency<<3)|(Dirty<<2)|(Valid<<1)|Global;
		set_C0_ENLO0(k1);

		opfn = page[3][i + 1].physical_pageframe_num;
		k1 = (opfn<<6)|(coherency<<3)|(Dirty<<2)|(Valid<<1)|Global;
		set_C0_ENLO1(k1);

		k1 = 0; 
		set_C0_PAGEMASK(k1);

		index_of_some_entry = i / 2;
		k1 = index_of_some_entry; 
		set_C0_INDEX(k1);

		set_tlb();
	}
}

void TLB_flush(){

	int k1;
	int vpn2;
	int asid;
	int epfn;
	int coherency;
	int opfn;
	int Dirty;
	int Valid;
	int Global;
	int index_of_some_entry;
	int i;
	for(i = 0; i < 64; i += 2)
	{
		vpn2 = 0 >> 1;
		asid = 0;
		k1 = (vpn2<<13)|(asid & 0xff);
		set_C0_ENHI(k1);

		coherency = 2; Dirty = 1; Global = 0;
		if(i == 0)
			Valid = 0;
		else 
			Valid = 0;

		epfn = 0x1800;
		k1 = (epfn<<6)|(coherency<<3)|(Dirty<<2)|(Valid<<1)|Global;
		set_C0_ENLO0(k1);

		opfn = 0x1801;
		k1 = (opfn<<6)|(coherency<<3)|(Dirty<<2)|(Valid<<1)|Global;
		set_C0_ENLO1(k1);

		k1 = 0; 
		set_C0_PAGEMASK(k1);

		index_of_some_entry = i / 2;
		k1 = index_of_some_entry; 
		set_C0_INDEX(k1);

		set_tlb();
	}
}

static void init_memory()
{
	init_page_table(); 
	//In task1&2, page table is initialized completely with address mapping, but only virtual pages in task3.
	//init_TLB();		//only used in P4 task1
	TLB_flush();
	physical_unused_num = 0x1000;
	tlb_unused_index = 0;
	pyhsical_page_full_flag = 0;
	disk_unused_num = 2;
	//init_swap();		//only used in P4 bonus: Page swap mechanism
}

static void init_pcb()
{
	queue_init(&ready_queue);
	queue_init(&block_queue);
	queue_init(&recv_wait_queue);

	stack_temp = STACK_BASE - STACK_SIZE; int i = 0;
	int count;
	process_id = 1;


	//init shell pcb
	pcb[1].user_stack_top = pcb[1].user_context.regs[29] = stack_temp;
	process_id = 1;
	pcb[1].pid = process_id++;
	pcb[1].user_context.regs[31] = pcb[1].user_context.pc = task_shell.entry_point;
	pcb[1].status = TASK_READY;
	pcb[1].user_context.cp0_status = 0x00008001;
	pcb[1].user_context.cp0_epc = pcb[i].user_context.regs[31];
	pcb[1].lock_count = 0;
	pcb[1].killed = 0;
	pcb[1].mapped = 0;
    queue_init(&(pcb[1].wait));
	stack_temp -= STACK_SIZE;
	queue_push(&ready_queue, &pcb[1]);
	/*
	for( i = 2; i < 3; ++i){
		pcb[i].user_stack_top = pcb[i].user_context.regs[29] = stack_temp;
		pcb[i].pid = process_id++;
		pcb[i].user_context.regs[31] = pcb[i].user_context.pc = test_tasks[i - 2]->entry_point;
		pcb[i].status = TASK_READY;
		pcb[i].user_context.cp0_status = 0x00008001;
		pcb[i].user_context.cp0_epc = pcb[i].user_context.regs[31];
		pcb[i].lock_count = 0;
		pcb[i].killed = 0;
	    queue_init(&(pcb[i].wait));
		stack_temp -= STACK_SIZE;
		queue_push(&ready_queue, &pcb[i]);
	}	*/
}

static void init_exception_handler()
{

}

static void init_exception()
{
	// 1. Get CP0_STATUS
	// 2. Disable all interrupt
	// 3. Copy the level 2 exception handling code to 0x80000180
	// 4. reset CP0_COMPARE & CP0_COUNT register
	printk("begin copy\n");
	copy_code();	//含有关中断操作
	copy_tlb_code(); 	//拷贝tlb例外处理代码至0x80000000
	int * copy_begin_ptr = exception_handler_begin;
	int * copy_end_ptr = exception_handler_end;
	printk("%x -- %x \n", copy_begin_ptr, copy_end_ptr);
	printk("copy %d bytes succeed\n", copy_end_ptr - copy_begin_ptr);
	reset_CPCOUNT();
}

static void init_syscall(void)
{
	// init system call table.
	syscall[SYSCALL_SLEEP] = do_sleep;
	syscall[SYSCALL_WRITE] = screen_write;
	syscall[SYSCALL_CURSOR] = screen_move_cursor;
	syscall[SYSCALL_MUTEX_LOCK_INIT] = do_mutex_lock_init;
	syscall[SYSCALL_MUTEX_LOCK_ACQUIRE] = do_mutex_lock_acquire;
	syscall[SYSCALL_MUTEX_LOCK_RELEASE] = do_mutex_lock_release;
	syscall[SYSCALL_PS] = do_ps;
	syscall[SYSCALL_EXIT] = do_exit;
	syscall[SYSCALL_WAIT] = do_wait;
	syscall[SYSCALL_SPAWN] = do_spawn;
	syscall[SYSCALL_KILL] = do_kill;
	syscall[SYSCALL_SEMAPHORE_INIT] = do_semaphore_init;
	syscall[SYSCALL_SEMAPHORE_UP] = do_semaphore_up;
	syscall[SYSCALL_SEMAPHORE_DOWN] = do_semaphore_down;
	syscall[SYSCALL_CONDITION_INIT] = do_condition_init;	
	syscall[SYSCALL_CONDITION_WAIT] = do_condition_wait;
	syscall[SYSCALL_CONDITION_SIGNAL] = do_condition_signal;
	syscall[SYSCALL_CONDITION_BROADCAST] = do_condition_broadcast;
	syscall[SYSCALL_BARRIER_INIT] = do_barrier_init;
	syscall[SYSCALL_BARRIER_WAIT] = do_barrier_wait;
	syscall[SYSCALL_GETPID] = do_getpid;

	syscall[SYSCALL_INIT_MAC] = do_init_mac;
	syscall[SYSCALL_NET_RECV] = do_net_recv;
	syscall[SYSCALL_NET_SEND] = do_net_send;
	syscall[SYSCALL_WAIT_RECV_PACKAGE] = do_wait_recv_package;

	syscall[SYSCALL_MKFS] = do_mkfs;
	syscall[SYSCALL_STATFS] = do_statfs;
	syscall[SYSCALL_LS] = do_ls;
	syscall[SYSCALL_MKDIR] = do_mkdir;
	syscall[SYSCALL_CD] = do_cd;
}

// jump from bootloader.
// The beginning of everything >_< ~~~~~~~~~~~~~~
void __attribute__((section(".entry_function"))) _start(void)
{
	// Close the cache, no longer refresh the cache 
	// when making the exception vector entry copy
	asm_start();
	current_running = 0;
	time_elapsed = 0;
	// init interrupt (^_^)
	init_exception();
	printk("> [INIT] Interrupt processing initialization succeeded.\n");

	// init virtual memory
	init_memory();
	printk("> [INIT] Virtual memory initialization succeeded.\n");

	// init system call table (0_0)
	init_syscall();
	printk("> [INIT] System call initialized successfully.\n");

	// init Process Control Block (-_-!)
	init_pcb();
	printk("> [INIT] PCB initialization succeeded.\n");

	// init screen (QAQ)
	init_screen();
	printk("> [INIT] SCREEN initialization succeeded.\n");
	screen_clear(0,0);
	enable_interrupt();

	init_scheduler();
	while (1)
	{
		// (QAQQQQQQQQQQQ)
		// If you do non-preemptive scheduling, you need to use it to surrender control
		//init_scheduler();
	};
	return;
}
