#include "cond.h"
#include "lock.h"

void do_condition_init(condition_t *condition)
{
    condition->numWaiting = 0;
    queue_init(&(condition->waitqueue));
}

void do_condition_wait(mutex_lock_t *lock, condition_t *condition)
{
    condition->numWaiting++;
    do_mutex_lock_release(lock);
    do_block(&(condition->waitqueue));
    //注意acquire的动作在syscall.c完成
}

void do_condition_signal(condition_t *condition)
{
    if(condition->numWaiting > 0){
        do_unblock_one(&(condition->waitqueue));
        condition->numWaiting--;
    }
}

void do_condition_broadcast(condition_t *condition)
{
    if(condition->numWaiting > 0){
        do_unblock_all(&(condition->waitqueue));
        condition->numWaiting = 0;
    }
}