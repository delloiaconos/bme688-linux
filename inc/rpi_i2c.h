#ifndef __RPI_I2C_H__
#define __RPI_I2C_H__

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>
#include <time.h>
#include <errno.h>

#include "bme68x.h" 
#ifdef __cplusplus
extern "C" {
#endif

// Struttura di contesto passata come intf_ptr alla Bosch API
typedef struct {
    int fd;           // file descriptor /dev/i2c-X
    uint8_t addr;     // indirizzo del dispositivo (0x76 o 0x77)
} rpi_i2c_ctx_t;


// WRITE: scrive reg_addr seguito da 'len' byte di payload
int8_t rpi_i2c_write(uint8_t reg_addr, const uint8_t *reg_data, uint32_t len, void *intf_ptr);
// READ: scrive reg_addr, poi legge 'len' byte
int8_t rpi_i2c_read(uint8_t reg_addr, uint8_t *reg_data, uint32_t len, void *intf_ptr);

#ifdef __cplusplus
}
#endif

#endif // __RPI_I2C_H__