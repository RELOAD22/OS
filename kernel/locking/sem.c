#include "sem.h"
#include "stdio.h"

void do_semaphore_init(semaphore_t *s, int val)
{
    s->sem = val;
    queue_init(&s->waitqueue);
}

//V()
void do_semaphore_up(semaphore_t *s)
{
    s->sem++;
    if(s->sem <= 0){
        do_unblock_one(&(s->waitqueue));
    }
}
//P()
void do_semaphore_down(semaphore_t *s)
{
    s->sem--;
    if(s->sem < 0){
        do_block(&(s->waitqueue));
    }
}