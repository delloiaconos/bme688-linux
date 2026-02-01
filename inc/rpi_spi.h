#ifndef __RPI_SPI_H__
#define __RPI_SPI_H__

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/spi/spidev.h>
#include <time.h>
#include <errno.h>

#include "bme68x.h"  // adjust include path

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    int fd;                    // /dev/spidevX.Y
    uint32_t speed_hz;         // e.g. 5000000
    uint8_t mode;              // SPI mode (0 or 3; use 0)
    uint8_t bits_per_word;     // 8
} rpi_spi_ctx_t;

// ---- SPI helpers -----------------------------------------------------------

//int rpi_spi_configure(int fd, uint8_t mode, uint8_t bits, uint32_t speed);
int rpi_spi_configure( rpi_spi_ctx_t * ctx );

// NOTE (BME68x SPI framing):
// - For *writes*: first byte = reg_addr & 0x7F, followed by payload bytes
// - For *reads* : first byte = reg_addr | 0x80, then clock out 'len' bytes
//   (no extra dummy byte needed)

int8_t rpi_spi_write(uint8_t reg_addr, const uint8_t *reg_data, uint32_t len, void *intf_ptr);
int8_t rpi_spi_read(uint8_t reg_addr, uint8_t *reg_data, uint32_t len, void *intf_ptr);

#ifdef __cplusplus
}
#endif

#endif // __RPI_SPI_H__