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
#include "common.h"
#include "syscall.h"
#include "time.h"
#include "shell.h"
//注意在shell.c中也有该宏定义
#define STACK_BASE  0xa0f00000
#define STACK_SIZE 0x100000

/* ready queue to run */
queue_t ready_queue;

/* block queue to wait */
queue_t block_queue;

/* current running task PCB */
pcb_t *current_running;
pid_t process_id;
pcb_t pcb[NUM_MAX_TASK];
int stack_temp;

extern uint32_t time_elapsed;	//time.c

static void init_pcb()
{
	queue_init(&ready_queue);
	queue_init(&block_queue);

	stack_temp = STACK_BASE; int i = 0;
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
