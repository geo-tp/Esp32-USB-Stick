
# USB Stick Emulator with SD Card Reader

This project transforms an SD card reader into a functional USB stick, the program exposes the SD card as a USB mass storage device, allowing it to be accessed and used directly from a computer or other USB host.

## Features

- **USB Mass Storage Device**: Enables the SD card to function as a standard USB stick.
- **Plug and Play**: Automatically mounts the SD card when connected to a USB host.

**Note:** Transfer speeds are quite low due to the limitation of the SD card reader in SPI mode

## Requirements

- **Hardware**:
  - M5Stack Cardputer
  - SD card
  - USB cable for power and data transfer

## Pin Configuration

The default pin configuration for the SD card reader is as follows:

| Pin Name     | GPIO  |
|--------------|-------|
| SD Card CS   | GPIO12 |
| SD Card MISO | GPIO39 |
| SD Card MOSI | GPIO14 |
| SD Card CLK  | GPIO40 |

## Usage

1. Insert an SD card into the reader.
2. Power on the ESP32-S3 and connect it to a USB host (e.g., a PC).
3. The SD card will appear as a USB mass storage device.
