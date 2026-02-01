#include <stdint.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <time.h>
#include <errno.h>
#include <linux/i2c-dev.h>

#include "rpi_bosch.h"
#include "bme68x.h" 


// registri TCA6408a
#define TCA6408A_REG_INPUT          0x00  // registro di input
#define TCA6408A_REG_OUTPUT         0x01  // registro di output
#define TCA6408A_REG_POLARITY       0x02  // registro polarity
#define TCA6408A_REG_CONFIG         0x03  // registro di configurazione

#define TCA6408A_REV_POLARITY       0xFF // reverse polarity
#define TCA6408A_CONF_AS_OUTPUT     0x00 // configura come output
#define TCA6408A_NO_SELECT          0x00 // nessuno selezionato

#define OUTPUT_IDLE_STATE           0xFF

/* I2C Helper: ensure slave address (idempotente, ma sicuro) */
inline int i2c_ensure_slave( rpi_i2c_ctx_t *ctx ) {
    if (ioctl(ctx->fd, I2C_SLAVE, ctx->addr) < 0) {
        return -1;
    } 
    return 0;
}

/* I2C Helper: write a register */
int i2c_write_reg( rpi_i2c_ctx_t *ctx, uint8_t reg, uint8_t val) {
    uint8_t buf[2] = { reg, val }; 
    ssize_t n = write(ctx->fd, buf, sizeof(buf));
    if (n != (ssize_t)sizeof(buf)) {
        return -1;
    }
    return 0;
}

/* I2C Helper: read a register */
int i2c_read_reg(rpi_i2c_ctx_t *ctx, uint8_t reg, uint8_t *val)
{
    if (!val) return -1;
    uint8_t buf[1] = { reg }; 

    /* Select register */
    ssize_t n = write(ctx->fd, buf, 1);
    if (n != 1) return -1;

    /* Read register value */
    n = read(ctx->fd, val, 1);
    if (n != 1) return -1;

    return 0;
}

/* SPI Helper: write a register */
int8_t spi_write(rpi_spi_ctx_t * ctx, uint8_t reg_addr, const uint8_t *reg_data, uint32_t len)
{
    if (!ctx || ctx->fd < 0) return BME68X_E_NULL_PTR;

    uint8_t header = reg_addr & 0x7F;

    // Compose [header][payload]
    uint8_t stack[32];
    uint8_t *buf = (len + 1 <= sizeof(stack)) ? stack : (uint8_t *)malloc(len + 1);
    if (!buf) return BME68X_E_COM_FAIL;

    buf[0] = header;
    if (len && reg_data) memcpy(&buf[1], reg_data, len);

    struct spi_ioc_transfer xfer = {
        .tx_buf = (unsigned long)buf,
        .rx_buf = 0,
        .len    = (uint32_t)(len + 1),
        .speed_hz = ctx->speed_hz,
        .bits_per_word = ctx->bits_per_word,
        .cs_change = 0,   // keep CS asserted only for this transfer
        .delay_usecs = 0,
    };

    int ret = ioctl(ctx->fd, SPI_IOC_MESSAGE(1), &xfer);
    if (buf != stack) free(buf);
    if (ret < 1) return BME68X_E_COM_FAIL;

    return BME68X_OK;
}


/* SPI Helper: read a register */
int8_t spi_read(rpi_spi_ctx_t * ctx, uint8_t reg_addr, uint8_t *reg_data, uint32_t len)
{
    if (!ctx || ctx->fd < 0 || !reg_data || len == 0) return BME68X_E_NULL_PTR;

    uint8_t header = reg_addr | 0x80; // read

    uint8_t tx[1 + 256]; // 1 header + up to 256 data bytes (adjust as needed)
    uint8_t rx[1 + 256];
    if (len > 256) return BME68X_E_COM_FAIL;

    tx[0] = header;
    memset(tx + 1, 0, len);

    struct spi_ioc_transfer xfer = {
        .tx_buf = (unsigned long)tx,
        .rx_buf = (unsigned long)rx,
        .len    = (uint32_t)(1 + len),
        .speed_hz = ctx->speed_hz,
        .bits_per_word = ctx->bits_per_word,
        .cs_change = 0,
        .delay_usecs = 0,
    };

    int ret = ioctl(ctx->fd, SPI_IOC_MESSAGE(1), &xfer);
    if (ret < 1) return BME68X_E_COM_FAIL;

    memcpy(reg_data, rx + 1, len); // skip header byte
    return BME68X_OK;
}


int rpi_bosch_configure( rpi_bosch_ctx_t * ctx )
{   
    /* CONFIGURE I2C DEVICE */
    if (ioctl(ctx->i2c_ctx->fd, I2C_SLAVE, ctx->i2c_ctx->addr) < 0) {
        fprintf(stderr, "ioctl(I2C_SLAVE, 0x%02X) failed: %s\n", ctx->i2c_ctx->addr, strerror(errno));
        close(ctx->i2c_ctx->fd);
        return -1;
    } 

    /* CONFIGURE SPI DEVICE */
    if (ioctl(ctx->spi_ctx->fd, SPI_IOC_WR_MODE, &(ctx->spi_ctx->mode)) < 0){
        return -1;
    }

    if (ioctl(ctx->spi_ctx->fd, SPI_IOC_WR_BITS_PER_WORD, &(ctx->spi_ctx->bits_per_word)) < 0) {
        return -1;
    }

    if (ioctl(ctx->spi_ctx->fd, SPI_IOC_WR_MAX_SPEED_HZ, &(ctx->spi_ctx->speed_hz)) < 0) {
        return -1;
    }

    /* TCA6480A OUTPUT Configuration.
       Sets ALL pins as outputs... 
       TODO: Change!
    */
    if (i2c_write_reg(ctx->i2c_ctx, TCA6408A_REG_CONFIG, 0x00) < 0) {
        fprintf(stderr, "write config failed: %s\n", strerror(errno));
        close(ctx->i2c_ctx->fd);
        return 1;
    }

    /* TCA6480A Input Polarity */
    if (i2c_write_reg(ctx->i2c_ctx, TCA6408A_REG_POLARITY, 0x00) < 0) {
        fprintf(stderr, "write config failed: %s\n", strerror(errno));
        close(ctx->i2c_ctx->fd);
        return 1;
    }

    if (i2c_write_reg(ctx->i2c_ctx, TCA6408A_REG_OUTPUT, OUTPUT_IDLE_STATE) < 0) {
        fprintf(stderr, "write config failed: %s\n", strerror(errno));
        close(ctx->i2c_ctx->fd);
        return 1;
    }

    return 0;
}

int8_t rpi_bosch_write(uint8_t reg_addr, const uint8_t *reg_data, uint32_t len, void *intf_ptr)
{
    int8_t ret;

    rpi_bosch_ctx_t *ctx = (rpi_bosch_ctx_t *) intf_ptr;

    if( !ctx || ctx->i2c_ctx->fd < 0 ) {
        return BME68X_E_NULL_PTR;
    } 
    
    if( i2c_ensure_slave(ctx->i2c_ctx) < 0 ) {
        return BME68X_E_COM_FAIL;
    }

    /* CHIP SELECT */
    if( i2c_write_reg(ctx->i2c_ctx, TCA6408A_REG_OUTPUT, ctx->cs_id) < 0 ) {
        fprintf(stderr, "write config failed: %s\n", strerror(errno));
        close(ctx->i2c_ctx->fd);
        return 1;
    }

    /* SPI READ */
    ret = spi_write( ctx->spi_ctx, reg_addr, reg_data, len );

    /* CHIP UNSELECT */
    if( i2c_write_reg(ctx->i2c_ctx, TCA6408A_REG_OUTPUT, OUTPUT_IDLE_STATE) < 0 ) {
        fprintf(stderr, "write config failed: %s\n", strerror(errno));
        close(ctx->i2c_ctx->fd);
        return 1;
    }

    return ret;
}


int8_t rpi_bosch_read(uint8_t reg_addr, uint8_t *reg_data, uint32_t len, void *intf_ptr)
{
    int8_t ret;

    rpi_bosch_ctx_t *ctx = (rpi_bosch_ctx_t *) intf_ptr;
    
    if( !ctx || ctx->i2c_ctx->fd < 0 ) {
        return BME68X_E_NULL_PTR;
    } 
    
    if( i2c_ensure_slave(ctx->i2c_ctx) < 0 ) {
        return BME68X_E_COM_FAIL;
    }

    /* CHIP SELECT */
    if( i2c_write_reg(ctx->i2c_ctx, TCA6408A_REG_OUTPUT, ctx->cs_id) < 0 ) {
        fprintf(stderr, "write config failed: %s\n", strerror(errno));
        close(ctx->i2c_ctx->fd);
        return 1;
    }

    /* SPI READ */
    ret = spi_read(  ctx->spi_ctx, reg_addr, reg_data, len );

    /* CHIP UNSELECT */
    if( i2c_write_reg(ctx->i2c_ctx, TCA6408A_REG_OUTPUT, OUTPUT_IDLE_STATE) < 0 ) {
        fprintf(stderr, "write config failed: %s\n", strerror(errno));
        close(ctx->i2c_ctx->fd);
        return 1;
    }
   
    return ret;
}
