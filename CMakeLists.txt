cmake_minimum_required(VERSION 3.22)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
#
# This file is generated only once,
# and is not re-generated if converter is called multiple times.
#
# User is free to modify the file as much as necessary
#

# Setup compiler settings
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS ON)


# Define the build type
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug")
endif()

# Set the project name
set(CMAKE_PROJECT_NAME f407_audio)

# Include toolchain file
include("cmake/gcc-arm-none-eabi.cmake")
include("cmake/stm32f407_hal.cmake")
include("cmake/middlewares.cmake")

# Enable compile command to ease indexing with e.g. clangd
set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)

# Core project settings
project(${CMAKE_PROJECT_NAME})
message("Build type: " ${CMAKE_BUILD_TYPE})

# Enable CMake support for ASM and C languages
enable_language(C ASM)

# Create an executable object type
add_executable(${CMAKE_PROJECT_NAME})

# Add some compile commond
target_compile_options(${CMAKE_PROJECT_NAME} PRIVATE
        -Wno-unused-parameter
        -Wno-pedantic
 )

# Link directories setup
target_link_directories(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user defined library search paths
)

file(GLOB USER_SRC 
    startup_stm32f407xx.s
    Drivers/BSP/wm8978/wm8978.c
    Drivers/BSP/delay/delay.c
    Drivers/BSP/sdio/sd_app.c
    Drivers/BSP/audio/audio.c
    Drivers/BSP/audio/decoder.c
    Drivers/BSP/audio/player.c
    Drivers/BSP/sram/sram_test.c
    Drivers/BSP/gt911/gt911.c
    Drivers/BSP/sw_i2c/sw_i2c.c
    Drivers/BSP/at24c/at24c.c
    Drivers/BSP/lcd/lcd.c
    Drivers/BSP/lcd/FONT.c
    User/Src/*.c
)

# Add sources to executable
target_sources(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user sources here
    ${STM32F407_HAL_SRC}
    ${MIDDLEWARES_SRC}
    ${USER_SRC}
)

# Add include paths
target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user defined include paths
    ${MIDDLEWARES_INC}
    ${STM32F407_HAL_INC}
    Drivers/BSP
    User/Inc
)

# Add project symbols (macros)
target_compile_definitions(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user defined symbols
	USE_HAL_DRIVER 
	STM32F407xx
    USE_USB_FS
    $<$<CONFIG:Debug>:DEBUG>
)

# Add linked libraries
target_link_libraries(${CMAKE_PROJECT_NAME}
    # Add user defined libraries
)

# Config project build time
# 获取当前时间
string(TIMESTAMP BUILD_TIME "%Y-%m-%d-%w %H:%M:%S")

# 生成头文件的路径
set(GENERATED_HEADER "${CMAKE_CURRENT_SOURCE_DIR}/User/Inc/bord_config.h")

# 配置头文件内容
configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/bord_config.h.in
    ${GENERATED_HEADER}
    @ONLY
)


# Generate HEX and BIN file
set(HEX_FILE ${PROJECT_BINARY_DIR}/${PROJECT_NAME}.hex)
set(BIN_FILE ${PROJECT_BINARY_DIR}/${PROJECT_NAME}.bin)
set(DSI_FILE ${PROJECT_BINARY_DIR}/${PROJECT_NAME}.dsi)

add_custom_command(
    TARGET ${PROJECT_NAME}
    POST_BUILD
    COMMAND ${CMAKE_SIZE} ${PROJECT_NAME}.elf
    COMMAND echo "Building ${HEX_FILE}"
    COMMAND echo "Building ${BIN_FILE}"
    # COMMAND echo "Building ${DSI_FILE}"
    COMMAND ${CMAKE_OBJCOPY} -Oihex   ${PROJECT_NAME}.elf ${HEX_FILE}
    COMMAND ${CMAKE_OBJCOPY} -Obinary ${PROJECT_NAME}.elf ${BIN_FILE}
    # COMMAND ${CMAKE_OBJDUMP} -D -S    ${PROJECT_NAME}.elf > ${DSI_FILE}
)