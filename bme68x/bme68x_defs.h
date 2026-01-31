/**
 * Copyright (C) 2021 Bosch Sensortec GmbH
 *
 * SPDX-License-Identifier: BSD-3-Clause
 * 
 */

#ifndef BME68X_DEFS_H_
#define BME68X_DEFS_H_

/********************************************************/
/* header includes */
#ifdef __KERNEL__
#include <linux/types.h>
#include <linux/kernel.h>
#else
#include <stdint.h>
#include <stddef.h>
#endif

/********************************************************/
/* macro definitions */

/* BME68X unique chip identifier */
#define BME68X_CHIP_ID  UINT8_C(0x61)

/* BME68X device address */
#define BME68X_I2C_ADDR_PRIMARY  UINT8_C(0x76)
#define BME68X_I2C_ADDR_SECONDARY  UINT8_C(0x77)

/* BME68X register addresses */
#define BME68X_REG_CHIP_ID  UINT8_C(0xd0)
#define BME68X_REG_VARIANT_ID  UINT8_C(0xf0)
#define BME68X_REG_SOFT_RESET  UINT8_C(0xe0)
#define BME68X_REG_CTRL_GAS_0  UINT8_C(0x70)
#define BME68X_REG_CTRL_GAS_1  UINT8_C(0x71)
#define BME68X_REG_CTRL_HUM  UINT8_C(0x72)
#define BME68X_REG_CTRL_MEAS  UINT8_C(0x74)
#define BME68X_REG_CONFIG  UINT8_C(0x75)

/* BME68X field data registers */
#define BME68X_REG_FIELD0  UINT8_C(0x1d)
#define BME68X_REG_MEAS_STATUS  UINT8_C(0x1d)

/* Coefficient registers */
#define BME68X_REG_COEFF1  UINT8_C(0x8a)
#define BME68X_REG_COEFF2  UINT8_C(0xe1)
#define BME68X_REG_COEFF3  UINT8_C(0x00)

/* Heater configuration registers */
#define BME68X_REG_RES_HEAT0  UINT8_C(0x5a)
#define BME68X_REG_GAS_WAIT0  UINT8_C(0x64)
#define BME68X_REG_IDAC_HEAT0  UINT8_C(0x50)

/* Soft reset command */
#define BME68X_SOFT_RESET_CMD  UINT8_C(0xb6)

/* Error codes */
#define BME68X_OK  INT8_C(0)
#define BME68X_E_NULL_PTR  INT8_C(-1)
#define BME68X_E_COM_FAIL  INT8_C(-2)
#define BME68X_E_DEV_NOT_FOUND  INT8_C(-3)
#define BME68X_E_INVALID_LENGTH  INT8_C(-4)
#define BME68X_W_DEFINE_OP_MODE  INT8_C(1)
#define BME68X_W_NO_NEW_DATA  INT8_C(2)

/* Oversampling settings */
#define BME68X_OS_NONE  UINT8_C(0)
#define BME68X_OS_1X  UINT8_C(1)
#define BME68X_OS_2X  UINT8_C(2)
#define BME68X_OS_4X  UINT8_C(3)
#define BME68X_OS_8X  UINT8_C(4)
#define BME68X_OS_16X  UINT8_C(5)

/* IIR filter settings */
#define BME68X_FILTER_OFF  UINT8_C(0)
#define BME68X_FILTER_SIZE_1  UINT8_C(1)
#define BME68X_FILTER_SIZE_3  UINT8_C(2)
#define BME68X_FILTER_SIZE_7  UINT8_C(3)
#define BME68X_FILTER_SIZE_15  UINT8_C(4)
#define BME68X_FILTER_SIZE_31  UINT8_C(5)
#define BME68X_FILTER_SIZE_63  UINT8_C(6)
#define BME68X_FILTER_SIZE_127  UINT8_C(7)

/* Operating modes */
#define BME68X_SLEEP_MODE  UINT8_C(0)
#define BME68X_FORCED_MODE  UINT8_C(1)
#define BME68X_PARALLEL_MODE  UINT8_C(2)
#define BME68X_SEQUENTIAL_MODE  UINT8_C(3)

/* Enable/Disable macros */
#define BME68X_ENABLE  UINT8_C(0x01)
#define BME68X_DISABLE  UINT8_C(0x00)

/* Variant ID macros */
#define BME68X_VARIANT_GAS_LOW  UINT8_C(0x00)
#define BME68X_VARIANT_GAS_HIGH  UINT8_C(0x01)

/********************************************************/
/* Type definitions */

/*!
 * Generic communication function pointer
 * @param[in] reg_addr: Register address
 * @param[out] data: Pointer to the data buffer to store the read data
 * @param[in] len: Number of bytes to read
 * @param[in,out] intf_ptr: Void pointer that can enable the linking of descriptors
 *                          for interface related call backs
 * @retval 0 for Success
 * @retval Non-zero for Failure
 */
typedef int8_t (*bme68x_read_fptr_t)(uint8_t reg_addr, uint8_t *data, uint32_t len, void *intf_ptr);

/*!
 * Generic communication function pointer
 * @param[in] reg_addr: Register address
 * @param[in] data: Pointer to the data buffer whose data has to be written
 * @param[in] len: Number of bytes to write
 * @param[in,out] intf_ptr: Void pointer that can enable the linking of descriptors
 *                          for interface related call backs
 * @retval 0 for Success
 * @retval Non-zero for Failure
 */
typedef int8_t (*bme68x_write_fptr_t)(uint8_t reg_addr, const uint8_t *data, uint32_t len, void *intf_ptr);

/*!
 * Delay function pointer
 * @param[in] period: Time period in microseconds
 * @param[in,out] intf_ptr: Void pointer that can enable the linking of descriptors
 *                          for interface related call backs
 */
typedef void (*bme68x_delay_us_fptr_t)(uint32_t period, void *intf_ptr);

/*!
 * Interface selection Enumerations
 */
enum bme68x_intf {
    BME68X_SPI_INTF,
    BME68X_I2C_INTF
};

/*!
 * @brief Structure to hold the calibration coefficients
 */
struct bme68x_calib_data {
    uint16_t par_h1;
    uint16_t par_h2;
    int8_t par_h3;
    int8_t par_h4;
    int8_t par_h5;
    uint8_t par_h6;
    int8_t par_h7;
    int8_t par_gh1;
    int16_t par_gh2;
    int8_t par_gh3;
    uint16_t par_t1;
    int16_t par_t2;
    int8_t par_t3;
    uint16_t par_p1;
    int16_t par_p2;
    int8_t par_p3;
    int16_t par_p4;
    int16_t par_p5;
    int8_t par_p6;
    int8_t par_p7;
    int16_t par_p8;
    int16_t par_p9;
    uint8_t par_p10;
    uint8_t res_heat_range;
    int8_t res_heat_val;
    int8_t range_sw_err;
};

/*!
 * @brief Structure to hold the TPH (Temperature, Pressure, Humidity) configuration
 */
struct bme68x_conf {
    uint8_t os_hum;
    uint8_t os_temp;
    uint8_t os_pres;
    uint8_t filter;
    uint8_t odr;
};

/*!
 * @brief Structure to hold the heater configuration
 */
struct bme68x_heatr_conf {
    uint8_t enable;
    uint16_t heatr_temp;
    uint16_t heatr_dur;
    uint8_t heatr_temp_prof[10];
    uint16_t heatr_dur_prof[10];
    uint8_t profile_len;
    uint8_t shared_heatr_dur;
};

/*!
 * @brief Structure to hold the sensor data
 */
struct bme68x_data {
    uint8_t status;
    uint8_t gas_index;
    uint8_t meas_index;
    int16_t temperature;
    uint32_t pressure;
    uint32_t humidity;
    uint32_t gas_resistance;
};

/*!
 * @brief BME68X device structure
 */
struct bme68x_dev {
    uint8_t chip_id;
    uint8_t variant_id;
    enum bme68x_intf intf;
    void *intf_ptr;
    int8_t amb_temp;
    struct bme68x_calib_data calib;
    struct bme68x_conf tph_sett;
    struct bme68x_heatr_conf gas_sett;
    uint8_t power_mode;
    uint8_t new_fields;
    bme68x_read_fptr_t read;
    bme68x_write_fptr_t write;
    bme68x_delay_us_fptr_t delay_us;
    int8_t info_msg;
};

#endif /* BME68X_DEFS_H_ */
