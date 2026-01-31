# BME688 Linux Driver

Linux userspace driver for the Bosch BME688 environmental sensor with gas sensing capability. This library provides a simple and efficient way to interface with the BME688 sensor on Linux systems, particularly Raspberry Pi.

## Features

- 🌡️ Temperature measurement (±1°C accuracy)
- 💧 Humidity measurement (±3% accuracy)
- 🌍 Pressure measurement (±1 hPa accuracy)
- 🔬 Gas resistance measurement (VOC detection)
- 🔌 I2C interface support
- 📦 Easy-to-use C library
- 🎯 Optimized for Raspberry Pi

## Hardware Requirements

- Raspberry Pi (or any Linux board with I2C support)
- BME688 sensor module
- Connection wires

### Wiring (I2C)

| BME688 Pin | Raspberry Pi Pin | Description |
|------------|------------------|-------------|
| VCC        | Pin 1 (3.3V)     | Power       |
| GND        | Pin 6 (GND)      | Ground      |
| SCL        | Pin 5 (GPIO3)    | I2C Clock   |
| SDA        | Pin 3 (GPIO2)    | I2C Data    |
| SDO        | GND or 3.3V      | I2C Address Select |

**Note:** Connect SDO to GND for I2C address 0x76, or to 3.3V for address 0x77.

## Software Requirements

- Linux (tested on Raspberry Pi OS)
- GCC compiler
- I2C development libraries

### Installing Dependencies

On Raspberry Pi / Debian-based systems:
```bash
sudo apt-get update
sudo apt-get install -y build-essential i2c-tools libi2c-dev
```

### Enable I2C Interface

On Raspberry Pi:
```bash
sudo raspi-config
# Navigate to: Interface Options -> I2C -> Enable
sudo reboot
```

Verify I2C is enabled:
```bash
ls /dev/i2c-*
# Should show: /dev/i2c-1
```

Detect BME688 sensor:
```bash
sudo i2cdetect -y 1
# Should show device at address 0x76 or 0x77
```

## Building the Library

Clone the repository and build:
```bash
git clone https://github.com/delloiaconos/bme688-linux.git
cd bme688-linux
make
```

This will:
- Build the static library (`lib/libbme68x.a`)
- Build the example application (`examples/basic_read`)

## Running the Example

```bash
sudo ./examples/basic_read
```

**Note:** Root privileges may be required to access I2C devices.

Sample output:
```
BME688 Sensor Example - Linux/Raspberry Pi
===========================================

BME688 initialized successfully (Chip ID: 0x61, Variant: 0x01)
Configuration successful!
Reading 10 samples...

Sample 1:
  Temperature:    23.45 °C
  Pressure:       1013.25 hPa
  Humidity:       45.20 %
  Gas Resistance: 125.50 KOhms
```

## API Usage

### Basic Example

```c
#include "src/bme68x_linux.h"

int main(void) {
    struct bme68x_dev dev;
    struct bme68x_conf conf;
    struct bme68x_heatr_conf heatr_conf;
    struct bme68x_data data;
    
    // Initialize sensor on I2C bus 1, address 0x76
    bme68x_linux_init(&dev, 1, BME68X_I2C_ADDR_PRIMARY);
    
    // Configure sensor
    conf.os_temp = BME68X_OS_2X;
    conf.os_pres = BME68X_OS_1X;
    conf.os_hum = BME68X_OS_16X;
    conf.filter = BME68X_FILTER_OFF;
    bme68x_set_conf(&conf, &dev);
    
    // Configure gas heater
    heatr_conf.enable = BME68X_ENABLE;
    heatr_conf.heatr_temp = 300;  // °C
    heatr_conf.heatr_dur = 100;   // ms
    bme68x_set_heatr_conf(&heatr_conf, &dev);
    
    // Trigger measurement
    bme68x_set_op_mode(BME68X_FORCED_MODE, &dev);
    usleep(250000);  // Wait for measurement
    
    // Read data
    bme68x_get_data(&data, &dev);
    
    printf("Temperature: %.2f °C\n", data.temperature / 100.0);
    printf("Pressure: %.2f hPa\n", data.pressure / 100.0);
    printf("Humidity: %.2f %%\n", data.humidity / 1000.0);
    printf("Gas: %.2f KOhms\n", data.gas_resistance / 1000.0);
    
    // Cleanup
    bme68x_linux_close(&dev);
    
    return 0;
}
```

## Project Structure

```
bme688-linux/
├── bme68x/              # BOSCH BME68X core library
│   ├── bme68x.c         # Core sensor API
│   ├── bme68x.h         # Public API header
│   └── bme68x_defs.h    # Definitions and constants
├── src/                 # Linux platform driver
│   ├── bme68x_linux.c   # I2C implementation
│   └── bme68x_linux.h   # Platform header
├── examples/            # Example applications
│   └── basic_read.c     # Basic sensor reading example
├── Makefile             # Build system
└── README.md            # This file
```

## Installation (System-wide)

To install the library system-wide:
```bash
sudo make install
```

This installs:
- Library: `/usr/local/lib/libbme68x.a`
- Headers: `/usr/local/include/bme68x/`

To use in your project:
```c
#include <bme68x/bme68x_linux.h>
```

Compile with:
```bash
gcc your_app.c -lbme68x -o your_app
```

## Troubleshooting

### I2C Device Not Found
- Check I2C is enabled: `ls /dev/i2c-*`
- Verify wiring connections
- Run `sudo i2cdetect -y 1` to detect devices
- Try the alternate I2C address (0x77 instead of 0x76)

### Permission Denied
- Run with `sudo` or add user to i2c group:
  ```bash
  sudo usermod -a -G i2c $USER
  # Log out and back in
  ```

### Compilation Errors
- Ensure `i2c-dev.h` is installed: `sudo apt-get install libi2c-dev`
- Check GCC version: `gcc --version` (requires GCC 4.8+)

## License

This project includes code from Bosch Sensortec's BME68x Sensor API.

- BME68X Core API: BSD-3-Clause (Copyright © 2021 Bosch Sensortec GmbH)
- Linux Platform Driver: BSD-3-Clause

See [LICENSE](LICENSE) for full details.

## Contributing

Contributions are welcome! Please feel free to submit issues or pull requests.

## References

- [BME688 Product Page](https://www.bosch-sensortec.com/products/environmental-sensors/gas-sensors/bme688/)
- [BME688 Datasheet](https://www.bosch-sensortec.com/media/boschsensortec/downloads/datasheets/bst-bme688-ds000.pdf)
- [Raspberry Pi I2C Documentation](https://www.raspberrypi.com/documentation/computers/os.html#i2c)

## Support

For issues and questions:
- Open an issue on GitHub
- Check the [Troubleshooting](#troubleshooting) section
- Review the [BME688 datasheet](https://www.bosch-sensortec.com/media/boschsensortec/downloads/datasheets/bst-bme688-ds000.pdf)
