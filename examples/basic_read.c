/**
 * BME688 Basic Example - Read Temperature, Pressure, Humidity, and Gas Resistance
 * 
 * This example demonstrates how to read sensor data from the BME688 on Linux/Raspberry Pi
 * 
 * Hardware connections (Raspberry Pi):
 * - VCC  -> 3.3V
 * - GND  -> GND
 * - SCL  -> GPIO3 (SCL)
 * - SDA  -> GPIO2 (SDA)
 * - SDO  -> GND (for I2C address 0x76) or 3.3V (for address 0x77)
 * 
 * Enable I2C on Raspberry Pi:
 *   sudo raspi-config -> Interface Options -> I2C -> Enable
 *   sudo reboot
 * 
 * Compile:
 *   make
 * 
 * Run:
 *   sudo ./examples/basic_read
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "../src/bme68x_linux.h"

int main(void)
{
    struct bme68x_dev dev;
    struct bme68x_conf conf;
    struct bme68x_heatr_conf heatr_conf;
    struct bme68x_data data;
    int8_t rslt;
    uint8_t n_samples = 10;
    
    printf("BME688 Sensor Example - Linux/Raspberry Pi\n");
    printf("===========================================\n\n");
    
    /* Initialize BME688 on I2C bus 1, address 0x76 */
    rslt = bme68x_linux_init(&dev, 1, BME68X_I2C_ADDR_PRIMARY);
    if (rslt != BME68X_OK) {
        fprintf(stderr, "Failed to initialize BME688\n");
        return EXIT_FAILURE;
    }
    
    /* Configure TPH (Temperature, Pressure, Humidity) settings */
    conf.filter = BME68X_FILTER_OFF;
    conf.odr = 0; /* ODR not used in forced mode */
    conf.os_hum = BME68X_OS_16X;
    conf.os_pres = BME68X_OS_1X;
    conf.os_temp = BME68X_OS_2X;
    
    rslt = bme68x_set_conf(&conf, &dev);
    if (rslt != BME68X_OK) {
        fprintf(stderr, "Failed to set configuration\n");
        bme68x_linux_close(&dev);
        return EXIT_FAILURE;
    }
    
    /* Configure heater for gas measurement */
    heatr_conf.enable = BME68X_ENABLE;
    heatr_conf.heatr_temp = 300;  /* degree Celsius */
    heatr_conf.heatr_dur = 100;   /* milliseconds */
    
    rslt = bme68x_set_heatr_conf(&heatr_conf, &dev);
    if (rslt != BME68X_OK) {
        fprintf(stderr, "Failed to set heater configuration\n");
        bme68x_linux_close(&dev);
        return EXIT_FAILURE;
    }
    
    printf("Configuration successful!\n");
    printf("Reading %d samples...\n\n", n_samples);
    
    /* Read sensor data in a loop */
    for (uint8_t i = 0; i < n_samples; i++) {
        /* Set forced mode to trigger measurement */
        rslt = bme68x_set_op_mode(BME68X_FORCED_MODE, &dev);
        if (rslt != BME68X_OK) {
            fprintf(stderr, "Failed to set forced mode\n");
            continue;
        }
        
        /* Wait for measurement to complete (typical: 150-200ms) */
        usleep(250000); /* 250ms */
        
        /* Read sensor data */
        rslt = bme68x_get_data(&data, &dev);
        if (rslt != BME68X_OK) {
            fprintf(stderr, "Failed to get sensor data\n");
            continue;
        }
        
        /* Check if new data is available */
        if (data.status & 0x80) {
            printf("Sample %d:\n", i + 1);
            printf("  Temperature:    %.2f °C\n", data.temperature / 100.0);
            printf("  Pressure:       %.2f hPa\n", data.pressure / 100.0);
            printf("  Humidity:       %.2f %%\n", data.humidity / 1000.0);
            printf("  Gas Resistance: %.2f KOhms\n", data.gas_resistance / 1000.0);
            printf("\n");
        } else {
            printf("Sample %d: No new data available\n\n", i + 1);
        }
        
        /* Wait before next measurement */
        sleep(1);
    }
    
    /* Cleanup */
    bme68x_linux_close(&dev);
    printf("BME688 sensor closed.\n");
    
    return EXIT_SUCCESS;
}
