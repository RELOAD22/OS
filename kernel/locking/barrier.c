#include "barrier.h"

void do_barrier_init(barrier_t *barrier, int goal)
{
    barrier->numBoundary = goal;
    barrier->numWaiting = 0;
    queue_init(&(barrier->waitqueue));
}

void do_barrier_wait(barrier_t *barrier)
{
    ++(barrier->numWaiting);
    if(barrier->numWaiting < barrier->numBoundary){
        do_block(&(barrier->waitqueue));
    }else if(barrier->numWaiting == barrier->numBoundary) {
        do_unblock_all(&(barrier->waitqueue));
        barrier->numWaiting = 0;
    }
}