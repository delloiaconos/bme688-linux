/**
 * Copyright (C) 2021 Bosch Sensortec GmbH
 *
 * SPDX-License-Identifier: BSD-3-Clause
 * 
 */

#ifndef BME68X_H_
#define BME68X_H_

#include "bme68x_defs.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initialize the BME68X sensor
 * 
 * @param[in,out] dev : Structure instance of bme68x_dev
 * 
 * @return Result of API execution status
 * @retval 0 -> Success
 * @retval < 0 -> Fail
 */
int8_t bme68x_init(struct bme68x_dev *dev);

/**
 * @brief Soft reset the sensor
 * 
 * @param[in,out] dev : Structure instance of bme68x_dev
 * 
 * @return Result of API execution status
 * @retval 0 -> Success
 * @retval < 0 -> Fail
 */
int8_t bme68x_soft_reset(struct bme68x_dev *dev);

/**
 * @brief Set the oversampling, filter and odr configuration
 * 
 * @param[in] conf : Desired sensor configuration
 * @param[in,out] dev : Structure instance of bme68x_dev
 * 
 * @return Result of API execution status
 * @retval 0 -> Success
 * @retval < 0 -> Fail
 */
int8_t bme68x_set_conf(const struct bme68x_conf *conf, struct bme68x_dev *dev);

/**
 * @brief Get the oversampling, filter and odr configuration
 * 
 * @param[out] conf : Current sensor configuration
 * @param[in,out] dev : Structure instance of bme68x_dev
 * 
 * @return Result of API execution status
 * @retval 0 -> Success
 * @retval < 0 -> Fail
 */
int8_t bme68x_get_conf(struct bme68x_conf *conf, struct bme68x_dev *dev);

/**
 * @brief Set the gas heater configuration
 * 
 * @param[in] conf : Desired heater configuration
 * @param[in,out] dev : Structure instance of bme68x_dev
 * 
 * @return Result of API execution status
 * @retval 0 -> Success
 * @retval < 0 -> Fail
 */
int8_t bme68x_set_heatr_conf(const struct bme68x_heatr_conf *conf, struct bme68x_dev *dev);

/**
 * @brief Set the operation mode of the sensor
 * 
 * @param[in] op_mode : Desired operation mode
 * @param[in,out] dev : Structure instance of bme68x_dev
 * 
 * @return Result of API execution status
 * @retval 0 -> Success
 * @retval < 0 -> Fail
 */
int8_t bme68x_set_op_mode(const uint8_t op_mode, struct bme68x_dev *dev);

/**
 * @brief Get the operation mode of the sensor
 * 
 * @param[out] op_mode : Current operation mode
 * @param[in,out] dev : Structure instance of bme68x_dev
 * 
 * @return Result of API execution status
 * @retval 0 -> Success
 * @retval < 0 -> Fail
 */
int8_t bme68x_get_op_mode(uint8_t *op_mode, struct bme68x_dev *dev);

/**
 * @brief Get the sensor data
 * 
 * @param[out] data : Structure instance to hold the sensor data
 * @param[in,out] dev : Structure instance of bme68x_dev
 * 
 * @return Result of API execution status
 * @retval 0 -> Success
 * @retval < 0 -> Fail
 */
int8_t bme68x_get_data(struct bme68x_data *data, struct bme68x_dev *dev);

#ifdef __cplusplus
}
#endif

#endif /* BME68X_H_ */
