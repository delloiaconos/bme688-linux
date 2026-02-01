#include "rpi.h"

void rpi_delay_us(uint32_t period, void *intf_ptr)
{
    (void)intf_ptr;
    struct timespec ts;
    ts.tv_sec  = period / 1000000u;
    ts.tv_nsec = (long)(period % 1000000u) * 1000L;
    nanosleep(&ts, NULL);
}
