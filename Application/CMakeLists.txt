cmake_minimum_required(VERSION 3.22)

# ------------------ Project Meta ------------------
set(CMAKE_PROJECT_NAME SecureBoot_F407)
project(${CMAKE_PROJECT_NAME} C ASM)

# ------------------ C Language Settings ------------------
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

# ------------------ Build Type ------------------
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build type" FORCE)
endif()
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")

# ------------------ Enable Tools ------------------
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# ------------------ Toolchain Setup ------------------
if(NOT DEFINED CMAKE_TOOLCHAIN_FILE)
    set(CMAKE_TOOLCHAIN_FILE "${CMAKE_SOURCE_DIR}/cmake/gcc-arm-none-eabi.cmake" CACHE FILEPATH "" FORCE)
endif()

include(${CMAKE_TOOLCHAIN_FILE})

# ------------------ Executable Target ------------------
add_executable(${CMAKE_PROJECT_NAME})

# ------------------ Add STM32CubeMX Code ------------------
add_subdirectory(cmake/stm32cubemx)

# ------------------ Linker Script ------------------
set(LINKER_SCRIPT "${CMAKE_SOURCE_DIR}/STM32F407VETx_APPLICATION.ld")
set(FLASH_ADDRESS 0x08010000)

target_compile_definitions(${CMAKE_PROJECT_NAME} PRIVATE
    APPLICATION_START_ADDRESS=0x08010000
    VECT_TAB_OFFSET=0x10000
)

target_link_options(${CMAKE_PROJECT_NAME} PRIVATE "-T${LINKER_SCRIPT}")

if(NOT EXISTS ${LINKER_SCRIPT})
    message(FATAL_ERROR "Linker script not found: ${LINKER_SCRIPT}")
endif()


# ------------------ Output File Generation ------------------

# Output .hex and .bin
add_custom_command(
    OUTPUT ${CMAKE_PROJECT_NAME}.hex ${CMAKE_PROJECT_NAME}.bin
    COMMAND ${CMAKE_OBJCOPY} -O ihex ${CMAKE_PROJECT_NAME}.elf ${CMAKE_PROJECT_NAME}.hex
    COMMAND ${CMAKE_OBJCOPY} -O binary ${CMAKE_PROJECT_NAME}.elf ${CMAKE_PROJECT_NAME}.bin
    DEPENDS ${CMAKE_PROJECT_NAME}
    COMMENT "Generating HEX and BIN files"
)

add_custom_target(binaries ALL
    DEPENDS ${CMAKE_PROJECT_NAME}.hex ${CMAKE_PROJECT_NAME}.bin
)

# ------------------ Flashing & Debug ------------------

add_custom_target(flash
    COMMAND STM32_Programmer_CLI -c port=SWD mode=UR -d ${CMAKE_PROJECT_NAME}.hex ${FLASH_ADDRESS}
    DEPENDS binaries
    COMMENT "Flashing firmware to STM32..."
)

add_custom_target(size
    COMMAND ${CMAKE_SIZE} ${CMAKE_PROJECT_NAME}.elf
    DEPENDS ${CMAKE_PROJECT_NAME}
    COMMENT "Displaying firmware size"
)

add_custom_target(debug
    COMMAND openocd -f interface/stlink.cfg -f target/stm32f4x.cfg
    COMMENT "Launching OpenOCD for STM32F4 debugging"
)

# ------------------ Cleanup ------------------

add_custom_target(clean_all
    COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --target clean
    COMMAND ${CMAKE_COMMAND} -E rm -rf ${CMAKE_BINARY_DIR}
    COMMENT "Cleaning all build artifacts"
)
