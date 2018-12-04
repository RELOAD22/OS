#include "lock.h"
#include "sched.h"
#include "syscall.h"
#include "shell.h"
#include "screen.h"

#define STACK_BASE  0xa0f00000
#define STACK_SIZE 0x100000

void do_ps(){
    pcb_t *pcd_check = ready_queue.head;
    int count = 0;

    vt100_move_cursor(screen_cursor_x, screen_cursor_y+1);
    if(ready_queue.head == NULL) {
        printk("no ready tasks in queue!");
        return;
    }    
    do{
        printk("[%d] pid : %d status: ",count, pcd_check->pid);
        if(pcd_check->status == TASK_READY)
            printk("READY\n");
        else 
            printk("RUNNING\n");
        pcd_check = pcd_check->next;
        count++;
        if(ready_queue.tail == ready_queue.head){
            return;
        }
    }while(pcd_check != ready_queue.tail);

    printk("[%d] pid : %d status: ",count, pcd_check->pid);
    if(pcd_check->status == TASK_READY)
        printk("READY\n");
    else 
        printk("RUNNING\n");
    screen_cursor_y += count;
}

void do_exit(){
    int i;
    //释放所有锁，以及所有因为这些锁被挂起的进程
    for(i = 0; i < current_running->lock_count; ++i){
        do_mutex_lock_release(current_running->lock[i]);
    }
    //将自身从ready队列移除
    current_running->killed = 1;
    //唤醒所有等待当前进程结束的进程
    do_unblock_all(&current_running->wait);
    scheduler();
}

void do_wait(int pid){
    if(pcb[pid].killed == 0){
        do_block(&(pcb[pid].wait));
    }
}

void do_spawn(task_info_t *task){
    int i = process_id;
	pcb[i].user_stack_top = pcb[i].user_context.regs[29] = stack_temp;
    if(i == 3)
        	pcb[i].user_stack_top = pcb[i].user_context.regs[29] = 0x000fff00;

	pcb[i].pid = process_id++;
	pcb[i].user_context.regs[31] = pcb[i].user_context.pc = task->entry_point;
	pcb[i].status = TASK_READY;
	pcb[i].user_context.cp0_status = 0x00008001;
	pcb[i].user_context.cp0_epc = pcb[i].user_context.regs[31];
	pcb[i].lock_count = 0;
	pcb[i].killed = 0;
	queue_init(&(pcb[i].wait));
	stack_temp -= STACK_SIZE;
	queue_push(&ready_queue, &pcb[i]);    
}

void do_kill(int pid){
    int i;
    //释放所有锁，以及所有因为这些锁被挂起的进程
    for(i = 0; i < pcb[pid].lock_count; ++i){
        do_mutex_lock_release(pcb[pid].lock[i]);
    }
    //从ready队列移除
    pcb[pid].killed = 1;
    //唤醒所有等待进程结束的进程
    do_unblock_all(&pcb[pid].wait);
}

int do_getpid(){
    return current_running->pid;
}