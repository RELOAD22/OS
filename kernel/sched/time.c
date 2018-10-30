#include "time.h"
#include "type.h"
uint32_t time_elapsed = 0;  //百万时钟周期为单位

static int MHZ = 300;

uint32_t get_ticks()
{
    return time_elapsed;
}

uint32_t get_timer()
{
    //return time_elapsed / (10000000);
    return time_elapsed / (300);
}

void latency(uint32_t time)
{
    uint32_t begin_time = get_timer();

    while (get_timer() - begin_time < time)
    {
    };
    return;
}