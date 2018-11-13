#include "lock.h"
#include "sched.h"
#include "syscall.h"
#include "shell.h"
#include "screen.h"

void do_ps(){
    pcb_t *pcd_check = ready_queue.head;
    int count = 0;

    vt100_move_cursor(screen_cursor_x, screen_cursor_y+1);
    if(ready_queue.head == NULL) {
        printk("no ready tasks in queue!");
        screen_cursor_y += 1;
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
            screen_cursor_y += count;
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

void do_wait(pid){
    if(pcb[pid].killed == 0){
        do_block(&(pcb[pid].wait));
    }
}