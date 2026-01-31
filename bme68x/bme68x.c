/**
 * Copyright (C) 2021 Bosch Sensortec GmbH
 *
 * SPDX-License-Identifier: BSD-3-Clause
 * 
 */

#include "bme68x.h"

/* Static function declarations */
static int8_t get_calib_data(struct bme68x_dev *dev);
static int8_t set_gas_config(const struct bme68x_heatr_conf *conf, struct bme68x_dev *dev);
static int8_t get_mem_page(struct bme68x_dev *dev);
static int8_t set_mem_page(uint8_t reg_addr, struct bme68x_dev *dev);
static int8_t boundary_check(uint8_t *value, uint8_t max, struct bme68x_dev *dev);
static int8_t null_ptr_check(const struct bme68x_dev *dev);
static int32_t calc_temperature(uint32_t temp_adc, struct bme68x_dev *dev);
static uint32_t calc_pressure(uint32_t pres_adc, const struct bme68x_dev *dev);
static uint32_t calc_humidity(uint16_t hum_adc, const struct bme68x_dev *dev);
static uint32_t calc_gas_resistance(uint16_t gas_res_adc, uint8_t gas_range, const struct bme68x_dev *dev);

/* Global variables */
static int32_t t_fine;

/**
 * @brief Initialize the BME68X sensor
 */
int8_t bme68x_init(struct bme68x_dev *dev)
{
    int8_t rslt;
    uint8_t chip_id;

    rslt = null_ptr_check(dev);
    if (rslt == BME68X_OK) {
        /* Read chip ID */
        rslt = dev->read(BME68X_REG_CHIP_ID, &chip_id, 1, dev->intf_ptr);
        if (rslt == BME68X_OK) {
            if (chip_id == BME68X_CHIP_ID) {
                dev->chip_id = chip_id;
                
                /* Read variant ID */
                rslt = dev->read(BME68X_REG_VARIANT_ID, &dev->variant_id, 1, dev->intf_ptr);
                
                if (rslt == BME68X_OK) {
                    /* Get calibration data */
                    rslt = get_calib_data(dev);
                }
            } else {
                rslt = BME68X_E_DEV_NOT_FOUND;
            }
        }
    }

    return rslt;
}

/**
 * @brief Soft reset the sensor
 */
int8_t bme68x_soft_reset(struct bme68x_dev *dev)
{
    int8_t rslt;
    uint8_t reg_data = BME68X_SOFT_RESET_CMD;

    rslt = null_ptr_check(dev);
    if (rslt == BME68X_OK) {
        rslt = dev->write(BME68X_REG_SOFT_RESET, &reg_data, 1, dev->intf_ptr);
        if (rslt == BME68X_OK) {
            /* Wait for 5ms for sensor to reset */
            dev->delay_us(5000, dev->intf_ptr);
        }
    }

    return rslt;
}

/**
 * @brief Set the sensor configuration
 */
int8_t bme68x_set_conf(const struct bme68x_conf *conf, struct bme68x_dev *dev)
{
    int8_t rslt;
    uint8_t odr_temp, odr_pres, odr_hum;
    uint8_t reg_data[4];

    rslt = null_ptr_check(dev);
    if ((rslt == BME68X_OK) && (conf != NULL)) {
        /* Validate input */
        odr_temp = conf->os_temp;
        odr_pres = conf->os_pres;
        odr_hum = conf->os_hum;
        
        rslt = boundary_check(&odr_temp, BME68X_OS_16X, dev);
        if (rslt == BME68X_OK)
            rslt = boundary_check(&odr_pres, BME68X_OS_16X, dev);
        if (rslt == BME68X_OK)
            rslt = boundary_check(&odr_hum, BME68X_OS_16X, dev);

        if (rslt == BME68X_OK) {
            /* Read current registers */
            rslt = dev->read(BME68X_REG_CTRL_HUM, reg_data, 4, dev->intf_ptr);
            
            if (rslt == BME68X_OK) {
                dev->power_mode = reg_data[3] & 0x03;
                
                /* Set humidity oversampling */
                reg_data[0] = (reg_data[0] & 0xf8) | odr_hum;
                
                /* Set temperature and pressure oversampling */
                reg_data[2] = (reg_data[2] & 0x1f) | (odr_temp << 5);
                reg_data[2] = (reg_data[2] & 0xe3) | (odr_pres << 2);
                
                /* Set filter */
                reg_data[3] = (reg_data[3] & 0xe3) | (conf->filter << 2);
                
                /* Write configuration */
                rslt = dev->write(BME68X_REG_CTRL_HUM, &reg_data[0], 1, dev->intf_ptr);
                if (rslt == BME68X_OK) {
                    rslt = dev->write(BME68X_REG_CONFIG, &reg_data[3], 1, dev->intf_ptr);
                    if (rslt == BME68X_OK) {
                        rslt = dev->write(BME68X_REG_CTRL_MEAS, &reg_data[2], 1, dev->intf_ptr);
                        
                        /* Store configuration */
                        dev->tph_sett = *conf;
                    }
                }
            }
        }
    } else {
        rslt = BME68X_E_NULL_PTR;
    }

    return rslt;
}

/**
 * @brief Get the sensor configuration
 */
int8_t bme68x_get_conf(struct bme68x_conf *conf, struct bme68x_dev *dev)
{
    int8_t rslt;
    uint8_t reg_data[4];

    rslt = null_ptr_check(dev);
    if ((rslt == BME68X_OK) && (conf != NULL)) {
        rslt = dev->read(BME68X_REG_CTRL_HUM, reg_data, 4, dev->intf_ptr);
        if (rslt == BME68X_OK) {
            conf->os_hum = reg_data[0] & 0x07;
            conf->filter = (reg_data[3] & 0x1c) >> 2;
            conf->os_temp = (reg_data[2] & 0xe0) >> 5;
            conf->os_pres = (reg_data[2] & 0x1c) >> 2;
            conf->odr = 0; /* ODR not used in forced mode */
        }
    } else {
        rslt = BME68X_E_NULL_PTR;
    }

    return rslt;
}

/**
 * @brief Set heater configuration
 */
int8_t bme68x_set_heatr_conf(const struct bme68x_heatr_conf *conf, struct bme68x_dev *dev)
{
    int8_t rslt;

    rslt = null_ptr_check(dev);
    if ((rslt == BME68X_OK) && (conf != NULL)) {
        rslt = set_gas_config(conf, dev);
        if (rslt == BME68X_OK) {
            dev->gas_sett = *conf;
        }
    } else {
        rslt = BME68X_E_NULL_PTR;
    }

    return rslt;
}

/**
 * @brief Set operation mode
 */
int8_t bme68x_set_op_mode(const uint8_t op_mode, struct bme68x_dev *dev)
{
    int8_t rslt;
    uint8_t reg_data;

    rslt = null_ptr_check(dev);
    if (rslt == BME68X_OK) {
        rslt = dev->read(BME68X_REG_CTRL_MEAS, &reg_data, 1, dev->intf_ptr);
        if (rslt == BME68X_OK) {
            reg_data = (reg_data & 0xfc) | (op_mode & 0x03);
            rslt = dev->write(BME68X_REG_CTRL_MEAS, &reg_data, 1, dev->intf_ptr);
            if (rslt == BME68X_OK) {
                dev->power_mode = op_mode;
            }
        }
    }

    return rslt;
}

/**
 * @brief Get operation mode
 */
int8_t bme68x_get_op_mode(uint8_t *op_mode, struct bme68x_dev *dev)
{
    int8_t rslt;
    uint8_t reg_data;

    rslt = null_ptr_check(dev);
    if ((rslt == BME68X_OK) && (op_mode != NULL)) {
        rslt = dev->read(BME68X_REG_CTRL_MEAS, &reg_data, 1, dev->intf_ptr);
        if (rslt == BME68X_OK) {
            *op_mode = reg_data & 0x03;
        }
    } else {
        rslt = BME68X_E_NULL_PTR;
    }

    return rslt;
}

/**
 * @brief Get sensor data
 */
int8_t bme68x_get_data(struct bme68x_data *data, struct bme68x_dev *dev)
{
    int8_t rslt;
    uint8_t buff[15];
    uint32_t temp_adc, pres_adc;
    uint16_t hum_adc, gas_adc;
    uint8_t gas_range;

    rslt = null_ptr_check(dev);
    if ((rslt == BME68X_OK) && (data != NULL)) {
        /* Read field data */
        rslt = dev->read(BME68X_REG_FIELD0, buff, 15, dev->intf_ptr);
        
        if (rslt == BME68X_OK) {
            data->status = buff[0] & 0x80;
            data->gas_index = buff[0] & 0x0f;
            data->meas_index = buff[1];
            
            /* Extract ADC values */
            pres_adc = (uint32_t)((uint32_t)buff[2] << 12) | ((uint32_t)buff[3] << 4) | ((uint32_t)buff[4] >> 4);
            temp_adc = (uint32_t)((uint32_t)buff[5] << 12) | ((uint32_t)buff[6] << 4) | ((uint32_t)buff[7] >> 4);
            hum_adc = (uint16_t)((uint16_t)buff[8] << 8) | buff[9];
            gas_adc = (uint16_t)((uint16_t)buff[13] << 2) | (buff[14] >> 6);
            gas_range = buff[14] & 0x0f;
            
            /* Calculate compensated values */
            data->temperature = calc_temperature(temp_adc, dev);
            data->pressure = calc_pressure(pres_adc, dev);
            data->humidity = calc_humidity(hum_adc, dev);
            data->gas_resistance = calc_gas_resistance(gas_adc, gas_range, dev);
        }
    } else {
        rslt = BME68X_E_NULL_PTR;
    }

    return rslt;
}

/* Static functions */

static int8_t get_calib_data(struct bme68x_dev *dev)
{
    int8_t rslt;
    uint8_t coeff_array[42];

    rslt = dev->read(BME68X_REG_COEFF1, coeff_array, 25, dev->intf_ptr);
    if (rslt == BME68X_OK) {
        rslt = dev->read(BME68X_REG_COEFF2, &coeff_array[25], 16, dev->intf_ptr);
    }
    
    if (rslt == BME68X_OK) {
        rslt = dev->read(BME68X_REG_COEFF3, &coeff_array[41], 1, dev->intf_ptr);
    }

    if (rslt == BME68X_OK) {
        /* Temperature coefficients */
        dev->calib.par_t1 = (uint16_t)(((uint16_t)coeff_array[34] << 8) | coeff_array[33]);
        dev->calib.par_t2 = (int16_t)(((int16_t)coeff_array[2] << 8) | coeff_array[1]);
        dev->calib.par_t3 = (int8_t)coeff_array[3];

        /* Pressure coefficients */
        dev->calib.par_p1 = (uint16_t)(((uint16_t)coeff_array[6] << 8) | coeff_array[5]);
        dev->calib.par_p2 = (int16_t)(((int16_t)coeff_array[8] << 8) | coeff_array[7]);
        dev->calib.par_p3 = (int8_t)coeff_array[9];
        dev->calib.par_p4 = (int16_t)(((int16_t)coeff_array[12] << 8) | coeff_array[11]);
        dev->calib.par_p5 = (int16_t)(((int16_t)coeff_array[14] << 8) | coeff_array[13]);
        dev->calib.par_p6 = (int8_t)coeff_array[16];
        dev->calib.par_p7 = (int8_t)coeff_array[15];
        dev->calib.par_p8 = (int16_t)(((int16_t)coeff_array[20] << 8) | coeff_array[19]);
        dev->calib.par_p9 = (int16_t)(((int16_t)coeff_array[22] << 8) | coeff_array[21]);
        dev->calib.par_p10 = (uint8_t)coeff_array[23];

        /* Humidity coefficients */
        dev->calib.par_h1 = (uint16_t)(((uint16_t)coeff_array[27] << 4) | (coeff_array[26] & 0x0f));
        dev->calib.par_h2 = (uint16_t)(((uint16_t)coeff_array[25] << 4) | (coeff_array[26] >> 4));
        dev->calib.par_h3 = (int8_t)coeff_array[28];
        dev->calib.par_h4 = (int8_t)coeff_array[29];
        dev->calib.par_h5 = (int8_t)coeff_array[30];
        dev->calib.par_h6 = (uint8_t)coeff_array[31];
        dev->calib.par_h7 = (int8_t)coeff_array[32];

        /* Gas heater coefficients */
        dev->calib.par_gh1 = (int8_t)coeff_array[37];
        dev->calib.par_gh2 = (int16_t)(((int16_t)coeff_array[36] << 8) | coeff_array[35]);
        dev->calib.par_gh3 = (int8_t)coeff_array[38];

        /* Other coefficients */
        dev->calib.res_heat_range = (coeff_array[41] & 0x30) >> 4;
        dev->calib.res_heat_val = (int8_t)coeff_array[0];
        dev->calib.range_sw_err = ((int8_t)(coeff_array[41] & 0xf0)) >> 4;
    }

    return rslt;
}

static int8_t set_gas_config(const struct bme68x_heatr_conf *conf, struct bme68x_dev *dev)
{
    int8_t rslt = BME68X_OK;
    uint8_t reg_data;
    
    if (conf->enable == BME68X_ENABLE) {
        /* Calculate heater resistance */
        uint8_t heatr_res = 0;
        int32_t var1;
        int32_t var2;
        int32_t var3;
        int32_t var4;
        int32_t var5;
        int32_t heatr_res_x100;

        var1 = (((int32_t)dev->amb_temp * dev->calib.par_gh3) / 1000) * 256;
        var2 = (dev->calib.par_gh1 + 784) * (((((dev->calib.par_gh2 + 154009) * conf->heatr_temp * 5) / 100) + 3276800) / 10);
        var3 = var1 + (var2 / 2);
        var4 = (var3 / (dev->calib.res_heat_range + 4));
        var5 = (131 * dev->calib.res_heat_val) + 65536;
        heatr_res_x100 = (int32_t)(((var4 / var5) - 250) * 34);
        heatr_res = (uint8_t)((heatr_res_x100 + 50) / 100);

        /* Write heater resistance */
        rslt = dev->write(BME68X_REG_RES_HEAT0, &heatr_res, 1, dev->intf_ptr);
        
        if (rslt == BME68X_OK) {
            /* Calculate gas wait time */
            uint8_t gas_wait = 0;
            uint32_t dur = conf->heatr_dur;
            uint8_t factor = 0;
            
            while (dur > 0x3F) {
                dur = dur / 4;
                factor += 1;
            }
            gas_wait = (uint8_t)(dur + (factor * 64));
            
            /* Write gas wait time */
            rslt = dev->write(BME68X_REG_GAS_WAIT0, &gas_wait, 1, dev->intf_ptr);
            
            if (rslt == BME68X_OK) {
                /* Enable gas measurement */
                rslt = dev->read(BME68X_REG_CTRL_GAS_1, &reg_data, 1, dev->intf_ptr);
                if (rslt == BME68X_OK) {
                    reg_data = (reg_data & 0xef) | (BME68X_ENABLE << 4);
                    rslt = dev->write(BME68X_REG_CTRL_GAS_1, &reg_data, 1, dev->intf_ptr);
                }
            }
        }
    } else {
        /* Disable gas measurement */
        rslt = dev->read(BME68X_REG_CTRL_GAS_1, &reg_data, 1, dev->intf_ptr);
        if (rslt == BME68X_OK) {
            reg_data = reg_data & 0xef;
            rslt = dev->write(BME68X_REG_CTRL_GAS_1, &reg_data, 1, dev->intf_ptr);
        }
    }

    return rslt;
}

static int8_t get_mem_page(struct bme68x_dev *dev)
{
    /* Not used in I2C mode */
    (void)dev;
    return BME68X_OK;
}

static int8_t set_mem_page(uint8_t reg_addr, struct bme68x_dev *dev)
{
    /* Not used in I2C mode */
    (void)reg_addr;
    (void)dev;
    return BME68X_OK;
}

static int8_t boundary_check(uint8_t *value, uint8_t max, struct bme68x_dev *dev)
{
    int8_t rslt = BME68X_OK;

    if (value != NULL) {
        if (*value > max) {
            *value = max;
            dev->info_msg = BME68X_W_DEFINE_OP_MODE;
        }
    } else {
        rslt = BME68X_E_NULL_PTR;
    }

    return rslt;
}

static int8_t null_ptr_check(const struct bme68x_dev *dev)
{
    int8_t rslt = BME68X_OK;

    if ((dev == NULL) || (dev->read == NULL) || (dev->write == NULL) || (dev->delay_us == NULL)) {
        rslt = BME68X_E_NULL_PTR;
    }

    return rslt;
}

static int32_t calc_temperature(uint32_t temp_adc, struct bme68x_dev *dev)
{
    int64_t var1;
    int64_t var2;
    int64_t var3;
    int32_t calc_temp;

    var1 = ((int32_t)temp_adc >> 3) - ((int32_t)dev->calib.par_t1 << 1);
    var2 = (var1 * (int32_t)dev->calib.par_t2) >> 11;
    var3 = ((var1 >> 1) * (var1 >> 1)) >> 12;
    var3 = ((var3) * ((int32_t)dev->calib.par_t3 << 4)) >> 14;
    t_fine = (int32_t)(var2 + var3);
    calc_temp = (((t_fine * 5) + 128) >> 8);

    return calc_temp;
}

static uint32_t calc_pressure(uint32_t pres_adc, const struct bme68x_dev *dev)
{
    int32_t var1;
    int32_t var2;
    int32_t var3;
    int32_t pressure_comp;

    var1 = (((int32_t)t_fine) >> 1) - 64000;
    var2 = ((((var1 >> 2) * (var1 >> 2)) >> 11) * (int32_t)dev->calib.par_p6) >> 2;
    var2 = var2 + ((var1 * (int32_t)dev->calib.par_p5) << 1);
    var2 = (var2 >> 2) + ((int32_t)dev->calib.par_p4 << 16);
    var1 = (((((var1 >> 2) * (var1 >> 2)) >> 13) * ((int32_t)dev->calib.par_p3 << 5)) >> 3) + (((int32_t)dev->calib.par_p2 * var1) >> 1);
    var1 = var1 >> 18;
    var1 = ((32768 + var1) * (int32_t)dev->calib.par_p1) >> 15;
    pressure_comp = 1048576 - pres_adc;
    pressure_comp = (int32_t)((pressure_comp - (var2 >> 12)) * ((uint32_t)3125));

    if (pressure_comp >= (1 << 30))
        pressure_comp = ((pressure_comp / (uint32_t)var1) << 1);
    else
        pressure_comp = ((pressure_comp << 1) / (uint32_t)var1);

    var1 = ((int32_t)dev->calib.par_p9 * (int32_t)(((pressure_comp >> 3) * (pressure_comp >> 3)) >> 13)) >> 12;
    var2 = ((int32_t)(pressure_comp >> 2) * (int32_t)dev->calib.par_p8) >> 13;
    var3 = ((int32_t)(pressure_comp >> 8) * (int32_t)(pressure_comp >> 8) * (int32_t)(pressure_comp >> 8) * (int32_t)dev->calib.par_p10) >> 17;

    pressure_comp = (int32_t)(pressure_comp) + ((var1 + var2 + var3 + ((int32_t)dev->calib.par_p7 << 7)) >> 4);

    return (uint32_t)pressure_comp;
}

static uint32_t calc_humidity(uint16_t hum_adc, const struct bme68x_dev *dev)
{
    int32_t var1;
    int32_t var2;
    int32_t var3;
    int32_t var4;
    int32_t var5;
    int32_t var6;
    int32_t temp_scaled;
    int32_t calc_hum;

    temp_scaled = (((int32_t)t_fine * 5) + 128) >> 8;
    var1 = (int32_t)(hum_adc - ((int32_t)((int32_t)dev->calib.par_h1 * 16))) - (((temp_scaled * (int32_t)dev->calib.par_h3) / ((int32_t)100)) >> 1);
    var2 = ((int32_t)dev->calib.par_h2 * (((temp_scaled * (int32_t)dev->calib.par_h4) / ((int32_t)100)) + (((temp_scaled * ((temp_scaled * (int32_t)dev->calib.par_h5) / ((int32_t)100))) >> 6) / ((int32_t)100)) + (int32_t)(1 << 14))) >> 10;
    var3 = var1 * var2;
    var4 = (int32_t)dev->calib.par_h6 << 7;
    var4 = ((var4) + ((temp_scaled * (int32_t)dev->calib.par_h7) / ((int32_t)100))) >> 4;
    var5 = ((var3 >> 14) * (var3 >> 14)) >> 10;
    var6 = (var4 * var5) >> 1;
    calc_hum = (((var3 + var6) >> 10) * ((int32_t)1000)) >> 12;

    if (calc_hum > 100000)
        calc_hum = 100000;
    else if (calc_hum < 0)
        calc_hum = 0;

    return (uint32_t)calc_hum;
}

static uint32_t calc_gas_resistance(uint16_t gas_res_adc, uint8_t gas_range, const struct bme68x_dev *dev)
{
    int64_t var1;
    uint64_t var2;
    int64_t var3;
    uint32_t calc_gas_res;
    uint32_t lookupTable1[16] = { 2147483647u, 2147483647u, 2147483647u, 2147483647u, 2147483647u, 2126008810u,
                                   2147483647u, 2130303777u, 2147483647u, 2147483647u, 2143188679u, 2136746228u,
                                   2147483647u, 2126008810u, 2147483647u, 2147483647u };
    uint32_t lookupTable2[16] = { 4096000000u, 2048000000u, 1024000000u, 512000000u, 255744255u, 127110228u,
                                   64000000u, 32258064u, 16016016u, 8000000u, 4000000u, 2000000u, 1000000u,
                                   500000u, 250000u, 125000u };

    var1 = (int64_t)((1340 + (5 * (int64_t)dev->calib.range_sw_err)) * ((int64_t)lookupTable1[gas_range])) >> 16;
    var2 = (((int64_t)((int64_t)gas_res_adc << 15) - (int64_t)(16777216)) + var1);
    var3 = (((int64_t)lookupTable2[gas_range] * (int64_t)var1) >> 9);
    calc_gas_res = (uint32_t)((var3 + ((int64_t)var2 >> 1)) / (int64_t)var2);

    return calc_gas_res;
}
