#include "lock.h"
#include "sched.h"
#include "syscall.h"
#include "shell.h"
#include "screen.h"

void do_ps(){
    pcb_t *pcd_check = ready_queue.head;
    int count = 0;
    if(ready_queue.head == NULL) return;
    vt100_move_cursor(screen_cursor_x, screen_cursor_y+1);
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