#ifndef INCLUDE_MM_H_
#define INCLUDE_MM_H_

#include "type.h"
#include "sched.h"

#define TLB_ENTRY_NUMBER 32

extern page_t page[NUM_MAX_TASK][256];

void do_TLB_Refill(int Context);
void do_page_fault();

#endif
