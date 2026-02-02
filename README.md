# bme688-linux
Bosch BME688 support on Linux/Raspberry PI

`bme688-linux` is a small, practical C/C++ project that makes Bosch **BME680/BME688** sensors easy to use on Linux—especially on **Raspberry Pi** without needing Arduino, kernel drivers, or heavy frameworks.
It integrates the [Bosch-BME68x-Library](https://github.com/boschsensortec/Bosch-BME68x-Library.git) (Arduino-style wrapper around the [BME68x Sensor API](https://github.com/boschsensortec/BME68x_SensorAPI.git)) and adds the missing Linux plumbing (SPI/I²C access + timing) so you can build a single executable and start streaming measurements immediately.

This project is designed to support:
- **Bosch BME688 Development Board with 8 sensors** (multi-sensor kit)
- **Single-sensor boards** from other vendors (e.g., Adafruit) with minimal adaptation

---

## Why this project

- **User-space only**: uses standard Linux interfaces (`/dev/spidev*`, `/dev/i2c-*`)
- **Fast to build**: simple `Makefile`, no CMake required
- **Multi-sensor ready**: example app targets the Bosch 8-sensor kit
- **Machine-friendly output**: prints one JSON object per measurement (easy piping to MQTT/InfluxDB/SQLite)

---

## What you get

- A working **Linux/RPi interface layer** (SPI, I²C, delays)
- An example app (`app.cpp`) that:
  - initializes **8 sensors**
  - configures **parallel mode** + heater profile
  - prints **timestamped JSON** lines continuously

Example output:
```json
{
  "idx": 0,
  "timestamp": 1769951541112,
  "measures": {
    "temperature": 32.302338,
    "pressure": 98724.414062,
    "humidity": 23.328861,
    "gas_resistance": 15964921,
    "gas_index": 3,
    "meas_index": 123,
    "idac": 98,
    "status": "0xB0"
  }
}
```

## Requirements

### Hardware
- Raspberry Pi with SPI and/or I²C enabled
- One of:
  - Bosch BME688 Development Board (multi-sensor kit), or
  - a single BME688/BME680 breakout board

### Software
- Raspberry Pi OS / Debian-like Linux
- Toolchain: `g++`, `gcc`, `make`

Install build tools:
```bash
sudo apt update
sudo apt install -y build-essential
```

Enable SPI/I²C (Raspberry Pi):
```bash
sudo raspi-config
# Interface Options -> enable SPI and/or I2C
```


## Build

If the Bosch library is included as a **git submodule**:
```bash
git submodule update --init --recursive
make
```

Otherwise, ensure the Bosch library sources are present in the expected project path, then run:
```bash
make
```


## Run

Build output is typically placed under `out/`. Example:
```bash
./out/bme-logger
```

If your app accepts device paths (common examples):
```bash
./out/bme-logger
```

## Supporting other boards (single sensor)

Many third-party boards expose:
- **I²C** 
- **SPI** (with a single CS pin)

The included Linux helpers (`rpi_i2c.*`, `rpi_spi.*`, delay helpers) are intended to be reusable so you can:
- switch transport (I²C/SPI)
- set the sensor count to `1`
- bypass any kit-specific chip-select logic if not needed


## Contribute

Contributions are welcome—especially improvements that make the project easier to build and run on Raspberry Pi and that broaden board compatibility while keeping the codebase lightweight.

You can contribute with:
- Support for additional BME688/BME680 breakout boards (I²C and SPI)
- Cleaner portability across Raspberry Pi models and Linux distros
- CLI improvements (device paths, sensor count, output format, sampling interval)
- Reliability improvements (error handling, reconnects, watchdog-friendly behavior)
- Documentation: wiring diagrams, troubleshooting notes, examples
- Output options: CSV, line protocol, MQTT-friendly formats

### How to contribute
1. Fork the repository
2. Create a feature branch:
   ```bash
   git checkout -b feature/my-change
   ```
3. Keep changes focused and documented:
   - avoid breaking existing Raspberry Pi builds
   - prefer small, readable patches
   - update `README.md` when behavior or usage changes
4. Build and run locally on a Raspberry Pi if possible
5. Open a pull request with:
   - what you changed and why
   - how you tested it (board type, interface, OS)

### Coding guidelines
- Keep the project **user-space only** and avoid heavy dependencies
- Follow existing formatting and naming conventions
- Prefer simple POSIX/Linux APIs (spidev, i2c-dev) over platform-specific hacks
- If adding new board support, keep kit-specific logic isolated


## License and attribution

This project **uses** the **Bosch-BME68x-Library** from **Bosch Sensortec**, which is licensed under **BSD-3-Clause**.  
Please retain Bosch Sensortec copyright and the BSD-3-Clause license text for that library (see the library’s own files).

Project license: see the repository LICENSE/COPYING file.
