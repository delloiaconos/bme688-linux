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

#include "rpi_spi.h"
#include "rpi_i2c.h"
#include "bme68x.h" 

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    rpi_spi_ctx_t   * spi_ctx;
    rpi_i2c_ctx_t   * i2c_ctx;
    uint8_t           cs_id;
} rpi_bosch_ctx_t;


// ---- SPI helpers -----------------------------------------------------------

int rpi_bosch_configure( rpi_bosch_ctx_t * ctx );

int8_t rpi_bosch_write(uint8_t reg_addr, const uint8_t *reg_data, uint32_t len, void *intf_ptr);
int8_t rpi_bosch_read(uint8_t reg_addr, uint8_t *reg_data, uint32_t len, void *intf_ptr);

#ifdef __cplusplus
}
#endif

#endif // __RPI_BOSCH_H__