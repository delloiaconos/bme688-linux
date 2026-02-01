#include "rpi_i2c.h"
#include "bme68x.h" 

#include <stdint.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <time.h>
#include <errno.h>

// Helper: imposta lo slave address (idempotente, ma sicuro)
inline int ensure_slave_addr(int fd, uint8_t addr) {
    if (ioctl(fd, I2C_SLAVE, addr) < 0) {
        return -1;
    } 
    return 0;
}

// WRITE: scrive reg_addr seguito da 'len' byte di payload
int8_t rpi_i2c_write(uint8_t reg_addr, const uint8_t *reg_data, uint32_t len, void *intf_ptr) {
    rpi_i2c_ctx_t *ctx = (rpi_i2c_ctx_t *)intf_ptr;
    if (!ctx || ctx->fd < 0) return BME68X_E_NULL_PTR;

    if (ensure_slave_addr(ctx->fd, ctx->addr) < 0) return BME68X_E_COM_FAIL;

    // buffer: [reg][data...]
    uint8_t stack_buf[32];
    uint8_t *buf = (len + 1 <= sizeof(stack_buf)) ? stack_buf : (uint8_t *)malloc(len + 1);
    if (!buf) return BME68X_E_COM_FAIL;

    buf[0] = reg_addr;
    if (len && reg_data) {
        memcpy(&buf[1], reg_data, len);
    }

    ssize_t w = write(ctx->fd, buf, len + 1);
    if (buf != stack_buf) free(buf);
    if (w != (ssize_t)(len + 1)) return BME68X_E_COM_FAIL;

    return BME68X_OK;
}

// READ: scrive reg_addr, poi legge 'len' byte
int8_t rpi_i2c_read(uint8_t reg_addr, uint8_t *reg_data, uint32_t len, void *intf_ptr) {
    rpi_i2c_ctx_t *ctx = (rpi_i2c_ctx_t *)intf_ptr;
    if (!ctx || ctx->fd < 0 || !reg_data) return BME68X_E_NULL_PTR;

    if (ensure_slave_addr(ctx->fd, ctx->addr) < 0) return BME68X_E_COM_FAIL;

    // seleziona il registro
    uint8_t reg = reg_addr;
    if (write(ctx->fd, &reg, 1) != 1) return BME68X_E_COM_FAIL;

    // leggi i dati
    ssize_t r = read(ctx->fd, reg_data, len);
    if (r != (ssize_t)len) return BME68X_E_COM_FAIL;

    return BME68X_OK;
}
