#ifndef __RPI_BOSCH_H__
#define __RPI_BOSCH_H__

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/spi/spidev.h>
#include <linux/i2c-dev.h>
#include <time.h>
#include <errno.h>

#include "bme68x.h"  // adjust include path
#include "rpi_spi.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    rpi_spi_ctx_t * spi_ctx;
    uint8_t         cs_id;
    int             i2c_fd;
    uint8_t         i2c_addr;
} rpi_bosch_ctx_t;


// ---- SPI helpers -----------------------------------------------------------

//int rpi_bosch_configure(int fd, uint8_t mode, uint8_t bits, uint32_t speed);
int rpi_bosch_configure( rpi_bosch_ctx_t * ctx );

// NOTE (BME68x SPI framing):
// - For *writes*: first byte = reg_addr & 0x7F, followed by payload bytes
// - For *reads* : first byte = reg_addr | 0x80, then clock out 'len' bytes
//   (no extra dummy byte needed)

int8_t rpi_bosch_write(uint8_t reg_addr, const uint8_t *reg_data, uint32_t len, void *intf_ptr);
int8_t rpi_bosch_read(uint8_t reg_addr, uint8_t *reg_data, uint32_t len, void *intf_ptr);

#ifdef __cplusplus
}
#endif

#endif // __RPI_BOSCH_H__