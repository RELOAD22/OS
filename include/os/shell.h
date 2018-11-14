
#ifndef INCLUDE_SHELL_H_
#define INCLUDE_SHELL_H_

#include "queue.h"
extern 

void do_ps(void);
void do_exit(void);
void do_wait(int);
void do_spawn(task_info_t *);
void do_kill(int);
int  do_getpid();

#endif
