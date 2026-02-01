#ifndef __RPI_H__
#define __RPI_H__

#include <stdint.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <time.h>
#include <errno.h>

#ifdef __cplusplus
extern "C" {
#endif

// Ritardo in microsecondi richiesto dalla Bosch API
void rpi_delay_us(uint32_t period, void *intf_ptr);

#ifdef __cplusplus
}
#endif

#endif //__RPI_H__
