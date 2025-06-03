cmake_minimum_required(VERSION 3.22)

# ------------------ Project Configuration ------------------

set(CMAKE_PROJECT_NAME SecureBoot_F407)
project(${CMAKE_PROJECT_NAME} C ASM)

# Use C11 standard
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Enable compile_commands.json for tools like clangd
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Set default build type
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif()
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")

# Include custom ARM GCC toolchain
include("cmake/gcc-arm-none-eabi.cmake")

# ------------------ Executable Target ------------------

add_executable(${CMAKE_PROJECT_NAME})

# Include STM32CubeMX-generated sources
add_subdirectory(cmake/stm32cubemx)

# ------------------ Linker Script Selection ------------------

# Choose appropriate linker script based on BOOTLOADER_BUILD
if(BOOTLOADER_BUILD)
    set(LINKER_SCRIPT "${CMAKE_SOURCE_DIR}/STM32F407VETx_BOOTLOADER.ld")
    target_compile_definitions(${CMAKE_PROJECT_NAME} PRIVATE BOOTLOADER)
    set(FLASH_ADDRESS 0x08000000)
else()
    set(LINKER_SCRIPT "${CMAKE_SOURCE_DIR}/STM32F407VETx_APPLICATION.ld")
    target_compile_definitions(${CMAKE_PROJECT_NAME} PRIVATE APPLICATION_START_ADDRESS=0x08010000)
    target_compile_definitions(${CMAKE_PROJECT_NAME} PRIVATE VECT_TAB_OFFSET=0x10000)
    set(FLASH_ADDRESS 0x08010000)
endif()

# Set linker script
target_link_options(${CMAKE_PROJECT_NAME} PRIVATE "-T${LINKER_SCRIPT}")

# ------------------ Flash & Debug Targets ------------------

# Generate .hex and .bin files
add_custom_command(
    OUTPUT ${CMAKE_PROJECT_NAME}.hex ${CMAKE_PROJECT_NAME}.bin
    COMMAND ${CMAKE_OBJCOPY} -O ihex ${CMAKE_PROJECT_NAME}.elf ${CMAKE_PROJECT_NAME}.hex
    COMMAND ${CMAKE_OBJCOPY} -O binary ${CMAKE_PROJECT_NAME}.elf ${CMAKE_PROJECT_NAME}.bin
    DEPENDS ${CMAKE_PROJECT_NAME}
    COMMENT "Generating .hex and .bin files"
)

add_custom_target(binaries ALL
    DEPENDS ${CMAKE_PROJECT_NAME}.hex ${CMAKE_PROJECT_NAME}.bin
)

# Flash target (flashes the correct address depending on boot/app)
add_custom_target(flash
    COMMAND ${CMAKE_OBJCOPY} -O ihex ${CMAKE_PROJECT_NAME}.elf ${CMAKE_PROJECT_NAME}.hex
    DEPENDS binaries
    COMMENT "Flashing to STM32 at ${FLASH_ADDRESS}"
)

# Show size of the firmware
add_custom_target(size
    COMMAND ${CMAKE_SIZE} ${CMAKE_PROJECT_NAME}.elf
    DEPENDS ${CMAKE_PROJECT_NAME}
    COMMENT "Displaying firmware size"
)

# Launch debugger (OpenOCD)
add_custom_target(debug
    COMMAND openocd -f interface/stlink.cfg -f target/stm32f4x.cfg
    COMMENT "Starting OpenOCD for STM32F4"
)

# ------------------ Clean All Artifacts ------------------

add_custom_target(clean_all
    COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --target clean
    COMMAND ${CMAKE_COMMAND} -E remove_directory ${CMAKE_BINARY_DIR}
    COMMENT "Full cleanup of build artifacts"
)
