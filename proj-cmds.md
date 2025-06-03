<!-- START doctoc generated TOC please keep comment here to allow auto update -->
<!-- DON'T EDIT THIS SECTION, INSTEAD RE-RUN doctoc TO UPDATE -->
**Table of Contents**  *generated with [DocToc](https://github.com/thlorenz/doctoc)*

- [🛠️ Build Instructions](#-build-instructions)
  - [⚙️ Bootloader](#-bootloader)

<!-- END doctoc generated TOC please keep comment here to allow auto update -->

## 🛠️ Build Instructions

### ⚙️ Bootloader

```bash
mkdir -p Bootloader/build
cd Bootloader/build
cmake .. -DBOOTLOADER_BUILD=ON
cmake --build .
make flash     # optional: flashes to 0x08000000

### Application

```bash
mkdir -p Application/build
cd Application/build
cmake .. -DBOOTLOADER_BUILD=OFF
cmake --build .
make flash     # optional: flashes to 0x08010000
```
