#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>
#include <time.h>
#include <errno.h>

#include "bme68xLibrary.h"
#include "bme68x.h" 

#include "rpi.h"
#include "rpi_bosch.h"

#define SPI_DEFAULT_DEV     "/dev/spidev0.0"
#define I2C_DEFAULT_DEV     "/dev/i2c-1"


#define N_KIT_SENS 8
#define MEAS_DUR 140

#define I2C_TCA6408A_SLAVE_ADDR     0x20

uint16_t tempProf[8][10] = { {320, 100, 100, 100, 200, 200, 200, 320, 320, 320},
                                      {320, 100, 100, 100, 200, 200, 200, 320, 320, 320},
                                      {320, 100, 100, 100, 200, 200, 200, 320, 320, 320},
                                      {320, 100, 100, 100, 200, 200, 200, 320, 320, 320},
                                      {320, 100, 100, 100, 200, 200, 200, 320, 320, 320},
                                      {320, 100, 100, 100, 200, 200, 200, 320, 320, 320},
                                      {320, 100, 100, 100, 200, 200, 200, 320, 320, 320},
                                      {320, 100, 100, 100, 200, 200, 200, 320, 320, 320}
                                    };
/* Multiplier to the shared heater duration */
uint16_t mulProf[8][10] = {  {5, 2, 10, 30, 5, 5, 5, 5, 5, 5},
                                      {5, 2, 10, 30, 5, 5, 5, 5, 5, 5},
                                      {5, 2, 10, 30, 5, 5, 5, 5, 5, 5},
                                      {5, 2, 10, 30, 5, 5, 5, 5, 5, 5},
                                      {5, 2, 10, 30, 5, 5, 5, 5, 5, 5},
                                      {5, 2, 10, 30, 5, 5, 5, 5, 5, 5},
                                      {5, 2, 10, 30, 5, 5, 5, 5, 5, 5},
                                      {5, 2, 10, 30, 5, 5, 5, 5, 5, 5}
                                    };

int main(int argc, char **argv) {

    Bme68x bme[N_KIT_SENS];
    bme68xData sensorData[N_KIT_SENS] = {0};
    uint32_t lastMeasindex[N_KIT_SENS] = {0};

    rpi_bosch_ctx_t ctx[N_KIT_SENS];

    const char *spi_dev = (argc > 1) ? argv[1] : SPI_DEFAULT_DEV;
    const char *i2c_dev = I2C_DEFAULT_DEV;

    int i2c_fd = open(i2c_dev, O_RDWR);
    if (i2c_fd < 0) { 
        perror("open"); 
        return 1; 
    }


    int spi_fd = open( spi_dev, O_RDWR);
    if (spi_fd < 0) { 
        perror("open"); 
        return 1; 
    }

    rpi_spi_ctx_t spi_ctx = {
        .fd = spi_fd,
        .speed_hz = 5000000,                // 5 MHz (BME68x supports up to 10 MHz)
        .mode = SPI_MODE_0 | SPI_NO_CS, 
        .bits_per_word = 8
    };

    rpi_i2c_ctx_t i2c_ctx = {
        .fd = i2c_fd,
        .addr = I2C_TCA6408A_SLAVE_ADDR
    };

    for (uint8_t i = 0; i < N_KIT_SENS; i++) {
        ctx[i].spi_ctx = &spi_ctx;
        ctx[i].i2c_ctx = &i2c_ctx;
        ctx[i].cs_id = 0xFF ^ (1 << i); // active low

        if (rpi_bosch_configure( &ctx[i] ) < 0) {
            perror("bosch_configure"); 
            close( i2c_fd ); 
            close( spi_fd ); 
            return 1;
        }
    }

    
    for (uint8_t i = 0; i < N_KIT_SENS; i++) {
        bme[i].begin( BME68X_SPI_INTF, rpi_bosch_read, rpi_bosch_write, rpi_delay_us, (void *) (&ctx[i]) );

        if(bme[i].checkStatus()) {
            printf("Initializing sensor %d failed with error\n", i);
        }
    }


    // Esempio rapido: leggere chip_id
    for (uint8_t i = 0; i < N_KIT_SENS; i++) {
        uint8_t chip_id = 0;

        chip_id = bme[i].readReg(BME68X_REG_CHIP_ID);

        printf( "{ 'idx' : %d,", i );
        printf( " 'chip_id' : 0x%02X }\n", chip_id );
    }


    for (uint8_t i = 0; i < N_KIT_SENS; i++) {
        bme[i].setTPH();

        // Shared heating duration in milliseconds 
        uint16_t sharedHeatrDur =
            MEAS_DUR - (bme[i].getMeasDur(BME68X_PARALLEL_MODE) / INT64_C(1000));

        bme[i].setHeaterProf(tempProf[i], mulProf[i], sharedHeatrDur, 10);

        // Parallel mode of sensor operation 
        bme[i].setOpMode(BME68X_PARALLEL_MODE);
    }

    while( 1 ) {
        int16_t indexDiff;
        
        sleep( 1 );
        for (uint8_t i = 0; i < N_KIT_SENS; i++) {
          if (bme[i].fetchData()) {
              uint8_t nFieldsLeft = bme[i].getData(sensorData[i]);

              /* Check if new data is received */
              if (sensorData[i].status & BME68X_NEW_DATA_MSK) {

                /* Inspect miss of data index */
                indexDiff = (int16_t)sensorData[i].meas_index - (int16_t)lastMeasindex[i];
                if (indexDiff > 1) {
                     lastMeasindex[i] = (int16_t)sensorData[i].meas_index;
                }
                lastMeasindex[i] = sensorData[i].meas_index;
                /*
                printf( "chip_idx       : %d\n", i );
                printf( "temperature    : %f\n", sensorData[i].temperature );
                printf( "pressure       : %f\n", sensorData[i].pressure );
                printf( "humidity       : %f\n", sensorData[i].humidity );
                printf( "gas_resistance : %f\n", sensorData[i].gas_resistance );
                printf( "gas_index      : %d\n", sensorData[i].gas_index );
                printf( "meas_index     : %d\n", sensorData[i].meas_index );
                printf( "idac           : %d\n", sensorData[i].idac );
                printf( "status         : %X\n", sensorData[i].status );
                */
                printf( "{ 'idx' : %d,", i );
                printf( " 'temperature' : %f,", sensorData[i].temperature );
                printf( " 'pressure' : %f,", sensorData[i].pressure );
                printf( " 'humidity' : %f,", sensorData[i].humidity );
                printf( " 'gas_resistance' : %f,", sensorData[i].gas_resistance );
                printf( " 'gas_index' : %d,", sensorData[i].gas_index );
                printf( " 'meas_index' : %d,", sensorData[i].meas_index );
                printf( " 'idac' : %d,", sensorData[i].idac );
                printf( " 'status' : %X }\n", sensorData[i].status );
              }
          }
        }
    }


    close(spi_fd);
    close(i2c_fd);
    return 0;
}

