/**
 * BME688 Linux I2C Platform Driver
 * 
 * This file provides the platform-specific I2C interface for the BME688 sensor
 * on Linux systems like Raspberry Pi.
 */

#ifndef BME68X_LINUX_H_
#define BME68X_LINUX_H_

#include "../bme68x/bme68x.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Linux I2C interface structure
 */
struct bme68x_linux_intf {
    int fd;              /* I2C file descriptor */
    uint8_t dev_addr;    /* I2C device address */
};

/**
 * @brief Initialize BME688 with Linux I2C interface
 * 
 * @param[in,out] dev : BME68X device structure
 * @param[in] i2c_bus : I2C bus number (e.g., 1 for /dev/i2c-1)
 * @param[in] dev_addr : I2C device address (0x76 or 0x77)
 * 
 * @return Result of initialization
 * @retval 0 -> Success
 * @retval < 0 -> Failure
 */
int8_t bme68x_linux_init(struct bme68x_dev *dev, uint8_t i2c_bus, uint8_t dev_addr);

/**
 * @brief Close the I2C interface
 * 
 * @param[in] dev : BME68X device structure
 */
void bme68x_linux_close(struct bme68x_dev *dev);

/**
 * @brief I2C read function for Linux
 */
int8_t bme68x_i2c_read(uint8_t reg_addr, uint8_t *data, uint32_t len, void *intf_ptr);

/**
 * @brief I2C write function for Linux
 */
int8_t bme68x_i2c_write(uint8_t reg_addr, const uint8_t *data, uint32_t len, void *intf_ptr);

/**
 * @brief Delay function for Linux (microseconds)
 */
void bme68x_delay_us(uint32_t period, void *intf_ptr);

#ifdef __cplusplus
}
#endif

#endif /* BME68X_LINUX_H_ */
