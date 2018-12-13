#include "lock.h"
#include "time.h"
#include "stdio.h"
#include "sched.h"
#include "queue.h"
#include "screen.h"
#include "mac.h"

//pcb_t pcb[NUM_MAX_TASK];

/* current running task PCB */
//pcb_t *current_running;

/* global process id */
//pid_t process_id = 1;
unsigned int wait_time;
unsigned int time_place;
typedef pcb_t item_t;
extern uint32_t time_elapsed;
extern int priority_weight[];

static void check_sleeping()
{    
    pcb_t *temp_pcb;
    pcb_t *temp_temp_pcb;

    if(queue_is_empty(&block_queue))
        return;

    for(temp_pcb = block_queue.head; temp_pcb != NULL; ){
        if(time_elapsed - temp_pcb->block_time > temp_pcb->sleep_time){
            temp_temp_pcb = queue_remove(&block_queue, temp_pcb);
            temp_pcb->status = TASK_READY;
	        queue_push(&ready_queue, temp_pcb);
            temp_pcb = temp_temp_pcb;
        }else{
            temp_pcb = temp_pcb->next;
        }
    }
}

static void check_recv_package()
{
    int count;
    pcb_t *temp_pcb;

    desc_t *desc_ptr = 0xa0f80000 + 16 * 63;
    if(queue_is_empty(&recv_wait_queue))
        return; 

    if(((desc_ptr->tdes0) & 0x80000000) == 0){
        temp_pcb = queue_dequeue(&recv_wait_queue);
        temp_pcb->status = TASK_READY;
	    queue_push(&ready_queue, temp_pcb);
    }
}
//轮转调度

void scheduler(void)
{
    if(current_running){
	    current_running->cursor_x = screen_cursor_x;
	    current_running->cursor_y = screen_cursor_y;
    }
    
    check_sleeping();
    //check_recv_package();

	if (current_running&&current_running->status != TASK_BLOCKED && current_running->killed == 0){
        queue_push(&ready_queue, current_running);
	    current_running->status = TASK_READY;
    }

    if(queue_is_empty(&ready_queue))
        return; 

    do{
        current_running = queue_dequeue(&ready_queue);
    }while(current_running->killed == 1);
	current_running->status = TASK_RUNNING;

    set_C0_ENHI(current_running->pid);
    /*
    if(current_running->mapped == 1){
        check_stack_to_tlb();
    }*/

	screen_cursor_x = current_running->cursor_x;
	screen_cursor_y = current_running->cursor_y;
}

void check_stack_to_tlb(){
    int tlb_Context = (((current_running->user_context).regs[29]) / 0x2000) * 0x10;

    int BadVnum = (tlb_Context / 0x10)*2;
	int vpn2 = BadVnum >> 1;

	int k1 = (vpn2<<13)|(current_running->pid  & 0xff);
	set_C0_ENHI(k1);
    find_tlbp(k1);	//查找C0_ENHI在tlb中

    //栈所在页面不在tlb中
    if(get_index() >= 0){
        do_TLB_Refill(tlb_Context);  
    }  
}
//优先级调度（动态调度），优先级内部实施轮转调度
/*
void scheduler(void)
{
    item_t * item = ready_queue.head;
    item_t * item_max_priority = ready_queue.head;
    int temp_priority = 0; int count = 0;
    check_sleeping();
    if(queue_is_empty(&ready_queue))
        return; 

	if (current_running&&current_running->status != TASK_BLOCKED){
        queue_push(&ready_queue, current_running);
	    current_running->status = TASK_READY;
    }

    for(;item != NULL; item = item->next){
        if(item->priority > temp_priority){
            temp_priority = item->priority;
            item_max_priority = item;
        }
    }
    if(temp_priority == 0){
	    for(count = 0; count <= NUM_MAX_TASK; count++){
		    pcb[count].priority = priority_weight[count];
	    }
        for(item = ready_queue.head;item != NULL; item = item->next){
            if(item->priority > temp_priority){
                temp_priority = item->priority;
                item_max_priority = item;
            }
        }
    }    
    item_max_priority->priority--;

    queue_remove(&ready_queue, item_max_priority);
    current_running = item_max_priority;
    current_running->status = TASK_RUNNING;
}*/

void do_sleep(int sleep_time)
{
    current_running->sleep_time = sleep_time * 300;
    current_running->block_time = time_elapsed;

    // block the current_running task into the queue
    queue_push(&block_queue, current_running);
	current_running->status = TASK_BLOCKED;
    //调用调度器，注意此时current所指的线程不会放至就绪队列
    scheduler();
}

void do_block(queue_t *queue)
{
    // block the current_running task into the queue
    queue_push(queue, current_running);
	current_running->status = TASK_BLOCKED;
    //调用调度器，注意此时current所指的线程不会放至就绪队列
    scheduler();
}

void do_unblock_one(queue_t *queue)
{
    // unblock the head task from the queue
	pcb_t *blockeditem = queue_dequeue(queue);
	blockeditem->status = TASK_READY;
	queue_push(&ready_queue, blockeditem);
}

void do_unblock_all(queue_t *queue)
{
    // unblock all task in the queue
    while(!queue_is_empty(queue)){
	    pcb_t *blockeditem = queue_dequeue(queue);
	    blockeditem->status = TASK_READY;
	    queue_push(&ready_queue, blockeditem);      
    }
}
