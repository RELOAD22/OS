#ifndef INCLUDE_MM_H_
#define INCLUDE_MM_H_

#include "type.h"
#include "sched.h"

#define TLB_ENTRY_NUMBER 32
#define PYHSICAL_PAGE_NUMBER 0X2000
#define PTE_NUM 0x100

extern int pyhsical_page_full_flag;
extern page_t page[NUM_MAX_TASK][256];
extern int tlb_unused_index;
extern int physical_unused_num;
extern int disk_unused_num;
void do_TLB_Refill(int Context);
void do_page_fault(int BadVnum);
void do_page_replace(int BadVnum);

#endif
