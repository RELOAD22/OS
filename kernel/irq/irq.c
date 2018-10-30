#include "irq.h"
#include "time.h"
#include "sched.h"
#include "string.h"

extern uint32_t time_elapsed;
static void irq_timer()
{
    // TODO clock interrupt handler.
    // scheduler, time counter in here to do, emmmmmm maybe.
    time_elapsed += 0x5000000;
    scheduler();
}

void interrupt_helper(uint32_t status, uint32_t cause)
{
    irq_timer();
    // TODO interrupt handler.
    // Leve3 exception Handler.
    // read CP0 register to analyze the type of interrupt.
}

void other_exception_handler()
{
    // TODO other exception handler
}