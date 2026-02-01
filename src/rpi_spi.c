#include "rpi_spi.h"
#include "bme68x.h" 

#include <stdint.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <time.h>
#include <errno.h>

/*
typedef struct {
    int fd;                    // /dev/spidevX.Y
    uint32_t speed_hz;         // e.g. 5000000
    uint8_t mode;              // SPI mode (0 or 3; use 0)
    uint8_t bits_per_word;     // 8
} rpi_spi_ctx_t;

int rpi_spi_configure_old(int fd, uint8_t mode, uint8_t bits, uint32_t speed)
{
    if (ioctl(fd, SPI_IOC_WR_MODE, &mode) < 0) return -1;
    if (ioctl(fd, SPI_IOC_WR_BITS_PER_WORD, &bits) < 0) return -1;
    if (ioctl(fd, SPI_IOC_WR_MAX_SPEED_HZ, &speed) < 0) return -1;
    return 0;
}
*/

int rpi_spi_configure( rpi_spi_ctx_t * ctx )
{
    if (ioctl(ctx->fd, SPI_IOC_WR_MODE, &(ctx->mode)) < 0){
        return -1;
    }

    if (ioctl(ctx->fd, SPI_IOC_WR_BITS_PER_WORD, &(ctx->bits_per_word)) < 0) {
        return -1;
    }

    if (ioctl(ctx->fd, SPI_IOC_WR_MAX_SPEED_HZ, &(ctx->speed_hz)) < 0) {
        return -1;
    }
    return 0;
}

// NOTE (BME68x SPI framing):
// - For *writes*: first byte = reg_addr & 0x7F, followed by payload bytes
// - For *reads* : first byte = reg_addr | 0x80, then clock out 'len' bytes
//   (no extra dummy byte needed)

int8_t rpi_spi_write(uint8_t reg_addr, const uint8_t *reg_data, uint32_t len, void *intf_ptr)
{
    rpi_spi_ctx_t *ctx = (rpi_spi_ctx_t *)intf_ptr;
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

int8_t rpi_spi_read(uint8_t reg_addr, uint8_t *reg_data, uint32_t len, void *intf_ptr)
{
    rpi_spi_ctx_t *ctx = (rpi_spi_ctx_t *)intf_ptr;
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
