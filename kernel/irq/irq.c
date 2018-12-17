#include "irq.h"
#include "time.h"
#include "sched.h"
#include "string.h"
#include "mac.h"

extern uint32_t time_elapsed;
static void irq_timer()
{
    // TODO clock interrupt handler.
    // scheduler, time counter in here to do, emmmmmm maybe.
    time_elapsed += 0x100;
    screen_reflush();
    scheduler();
}

void interrupt_helper(uint32_t status, uint32_t cause)
{
    int IP = (cause & 0xff00) >> 8;

    
    if(IP == 0x8){
        irq_mac();
        vt100_move_cursor(1,10);
        printk("irq_mac success");
        vt100_move_cursor(1,11);
        printk("status:%08x cause:%08x", status, cause);        
    }
    else
        irq_timer();
    //irq_timer();
    // TODO interrupt handler.
    // Leve3 exception Handler.
    // read CP0 register to analyze the type of interrupt.
}

void other_exception_handler()
{
    // TODO other exception handler
}