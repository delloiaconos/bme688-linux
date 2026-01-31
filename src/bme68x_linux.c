/**
 * BME688 Linux I2C Platform Driver Implementation
 * 
 * This file implements the platform-specific I2C interface for the BME688 sensor
 * on Linux systems like Raspberry Pi.
 */

#include "bme68x_linux.h"
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>
#include <time.h>
#include <string.h>
#include <errno.h>

/**
 * @brief I2C read function for Linux
 */
int8_t bme68x_i2c_read(uint8_t reg_addr, uint8_t *data, uint32_t len, void *intf_ptr)
{
    struct bme68x_linux_intf *intf = (struct bme68x_linux_intf *)intf_ptr;
    
    if (intf == NULL || data == NULL) {
        return BME68X_E_NULL_PTR;
    }
    
    /* Write register address */
    if (write(intf->fd, &reg_addr, 1) != 1) {
        fprintf(stderr, "Failed to write register address: %s\n", strerror(errno));
        return BME68X_E_COM_FAIL;
    }
    
    /* Read data */
    if (read(intf->fd, data, len) != (ssize_t)len) {
        fprintf(stderr, "Failed to read data: %s\n", strerror(errno));
        return BME68X_E_COM_FAIL;
    }
    
    return BME68X_OK;
}

/**
 * @brief I2C write function for Linux
 */
int8_t bme68x_i2c_write(uint8_t reg_addr, const uint8_t *data, uint32_t len, void *intf_ptr)
{
    struct bme68x_linux_intf *intf = (struct bme68x_linux_intf *)intf_ptr;
    uint8_t *buf;
    int8_t rslt = BME68X_OK;
    
    if (intf == NULL || data == NULL) {
        return BME68X_E_NULL_PTR;
    }
    
    /* Allocate buffer for register address + data */
    buf = (uint8_t *)malloc(len + 1);
    if (buf == NULL) {
        return BME68X_E_COM_FAIL;
    }
    
    /* Prepare buffer: [reg_addr][data...] */
    buf[0] = reg_addr;
    memcpy(&buf[1], data, len);
    
    /* Write to I2C */
    if (write(intf->fd, buf, len + 1) != (ssize_t)(len + 1)) {
        fprintf(stderr, "Failed to write data: %s\n", strerror(errno));
        rslt = BME68X_E_COM_FAIL;
    }
    
    free(buf);
    return rslt;
}

/**
 * @brief Delay function for Linux (microseconds)
 */
void bme68x_delay_us(uint32_t period, void *intf_ptr)
{
    (void)intf_ptr; /* Unused parameter */
    
    struct timespec ts;
    ts.tv_sec = period / 1000000;
    ts.tv_nsec = (period % 1000000) * 1000;
    nanosleep(&ts, NULL);
}

/**
 * @brief Initialize BME688 with Linux I2C interface
 */
int8_t bme68x_linux_init(struct bme68x_dev *dev, uint8_t i2c_bus, uint8_t dev_addr)
{
    char i2c_dev[20];
    struct bme68x_linux_intf *intf;
    int8_t rslt;
    
    if (dev == NULL) {
        return BME68X_E_NULL_PTR;
    }
    
    /* Allocate interface structure */
    intf = (struct bme68x_linux_intf *)malloc(sizeof(struct bme68x_linux_intf));
    if (intf == NULL) {
        fprintf(stderr, "Failed to allocate interface structure\n");
        return BME68X_E_COM_FAIL;
    }
    
    /* Open I2C device */
    snprintf(i2c_dev, sizeof(i2c_dev), "/dev/i2c-%d", i2c_bus);
    intf->fd = open(i2c_dev, O_RDWR);
    if (intf->fd < 0) {
        fprintf(stderr, "Failed to open %s: %s\n", i2c_dev, strerror(errno));
        free(intf);
        return BME68X_E_COM_FAIL;
    }
    
    /* Set I2C slave address */
    if (ioctl(intf->fd, I2C_SLAVE, dev_addr) < 0) {
        fprintf(stderr, "Failed to set I2C slave address: %s\n", strerror(errno));
        close(intf->fd);
        free(intf);
        return BME68X_E_COM_FAIL;
    }
    
    intf->dev_addr = dev_addr;
    
    /* Configure BME68X device structure */
    dev->intf = BME68X_I2C_INTF;
    dev->read = bme68x_i2c_read;
    dev->write = bme68x_i2c_write;
    dev->delay_us = bme68x_delay_us;
    dev->intf_ptr = intf;
    dev->amb_temp = 25; /* Ambient temperature for heater calculation */
    
    /* Initialize the sensor */
    rslt = bme68x_init(dev);
    if (rslt != BME68X_OK) {
        fprintf(stderr, "BME68X initialization failed: %d\n", rslt);
        close(intf->fd);
        free(intf);
        return rslt;
    }
    
    printf("BME688 initialized successfully (Chip ID: 0x%02X, Variant: 0x%02X)\n", 
           dev->chip_id, dev->variant_id);
    
    return BME68X_OK;
}

/**
 * @brief Close the I2C interface
 */
void bme68x_linux_close(struct bme68x_dev *dev)
{
    if (dev != NULL && dev->intf_ptr != NULL) {
        struct bme68x_linux_intf *intf = (struct bme68x_linux_intf *)dev->intf_ptr;
        if (intf->fd >= 0) {
            close(intf->fd);
        }
        free(intf);
        dev->intf_ptr = NULL;
    }
}
