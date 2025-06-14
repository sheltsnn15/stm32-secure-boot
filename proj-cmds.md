<!-- START doctoc generated TOC please keep comment here to allow auto update -->
<!-- DON'T EDIT THIS SECTION, INSTEAD RE-RUN doctoc TO UPDATE -->
**Table of Contents**  *generated with [DocToc](https://github.com/thlorenz/doctoc)*

- [Build Instructions](#build-instructions)
  - [Bootloader](#bootloader)

<!-- END doctoc generated TOC please keep comment here to allow auto update -->

## Build Instructions

### Bootloader

```bash

### Bootloader
cd Bootloader
mkdir build
cd build
cmake .. -DCMAKE_TOOLCHAIN_FILE=../cmake/gcc-arm-none-eabi.cmake
cmake --build .
cmake --build . --target flash    # flashes to 0x08000000

### Application

cd Application 
mkdir build
cd build
cmake .. -DCMAKE_TOOLCHAIN_FILE=../cmake/gcc-arm-none-eabi.cmake
cmake --build .
cmake --build . --target flash    # optional: flashes to 0x08010000
```
