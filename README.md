# Secure Boot & Firmware Integrity on STM32

[![License](https://img.shields.io/badge/License-MIT-blue.svg)](https://opensource.org/licenses/MIT)

Implementation of a secure boot mechanism for STM32F407VET6 that verifies firmware integrity using SHA-256 hashes.

## Features

- Minimal bootloader with application jump
- SHA-256 firmware verification
- Tamper detection
- UART/LED status indicators

## Requirements

- Hardware:
  - STM32F407VET6 board
  - ST-Link V2 programmer
- Software:
  - STM32CubeIDE or `arm-none-eabi-gcc` toolchain
  - OpenSSL (for hash generation)
  - Python (optional for scripts)

## Memory Layout

| Address Range    | Content          |
|------------------|------------------|
| 0x08000000-0x0800FFFF | Bootloader       |
| 0x08010000-...   | Application Firmware |
| Last 32 bytes    | Stored SHA-256   |

## Building and Flashing

1. Generate firmware hash:

```bash
openssl dgst -sha256 -binary firmware.bin | dd of=firmware_with_hash.bin bs=1 seek=$(stat -c%s firmware.bin) conv=notrunc
