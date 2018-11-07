#include "lock.h"
#include "sched.h"
#include "syscall.h"

void spin_lock_init(spin_lock_t *lock)
{
    lock->status = UNLOCKED;
}

void spin_lock_acquire(spin_lock_t *lock)
{
    while (LOCKED == lock->status)
    {
    };
    lock->status = LOCKED;
}

void spin_lock_release(spin_lock_t *lock)
{
    lock->status = UNLOCKED;
}

void do_mutex_lock_init(mutex_lock_t *lock)
{
	lock->status = UNLOCKED;  
    queue_init(&lock->lock_queue);   
}

void do_mutex_lock_acquire(mutex_lock_t *lock)
{	
    //有锁加入阻塞队列
	if (LOCKED == lock->status){
		do_block(&lock->lock_queue);
        return;
	}else{
	//无锁则上锁
		current_running->lock = lock;
	    lock->status = LOCKED;  
    } 
}

void do_mutex_lock_release(mutex_lock_t *lock)
{
    //lock->status = UNLOCKED;
    if (!queue_is_empty(&lock->lock_queue))	{
		do_unblock_one(&lock->lock_queue);
    }
	lock->status = UNLOCKED;
    current_running->lock = NULL;
}
