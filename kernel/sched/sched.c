#include "lock.h"
#include "time.h"
#include "stdio.h"
#include "sched.h"
#include "queue.h"
#include "screen.h"

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
    if(time_elapsed - time_place > wait_time && !queue_is_empty(&block_queue)){
        pcb_t *blockeditem = queue_dequeue(&block_queue);
	    blockeditem->status = TASK_READY;
	    queue_push(&ready_queue, blockeditem);
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

	if (current_running&&current_running->status != TASK_BLOCKED && current_running->killed == 0){
        queue_push(&ready_queue, current_running);
	    current_running->status = TASK_READY;
    }

    if(queue_is_empty(&ready_queue))
        return; 

    current_running = queue_dequeue(&ready_queue);
	current_running->status = TASK_RUNNING;

	screen_cursor_x = current_running->cursor_x;
	screen_cursor_y = current_running->cursor_y;
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
    wait_time = sleep_time * 300;
    time_place = time_elapsed;
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
