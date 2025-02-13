
set(STM32F407_HAL_DIR "${CMAKE_SOURCE_DIR}/Drivers/STM32F4xx_HAL_Driver")

set(STM32F407_HAL_SRC
    stm32f4xx_hal.c
    stm32f4xx_hal_cortex.c
    stm32f4xx_hal_dma.c
    stm32f4xx_hal_dma_ex.c
    stm32f4xx_hal_flash_ex.c
    stm32f4xx_hal_flash.c
    stm32f4xx_hal_gpio.c
    stm32f4xx_hal_i2c_ex.c
    stm32f4xx_hal_i2c.c
    stm32f4xx_hal_i2s_ex.c
    stm32f4xx_hal_i2s.c
    stm32f4xx_hal_pwr.c
    stm32f4xx_hal_pwr_ex.c
    stm32f4xx_hal_rcc.c
    stm32f4xx_hal_rcc_ex.c
    stm32f4xx_hal_rng.c
    stm32f4xx_hal_rtc.c
    stm32f4xx_hal_rtc_ex.c
    stm32f4xx_hal_sd.c
    stm32f4xx_hal_spi.c
    stm32f4xx_hal_sram.c
    stm32f4xx_hal_tim_ex.c
    stm32f4xx_hal_tim.c
    stm32f4xx_hal_uart.c
    stm32f4xx_hal_usart.c
    stm32f4xx_hal_pcd_ex.c
    stm32f4xx_hal_pcd.c
    stm32f4xx_ll_usb.c
    stm32f4xx_ll_fsmc.c
    stm32f4xx_ll_sdmmc.c
)

list(TRANSFORM STM32F407_HAL_SRC PREPEND "${STM32F407_HAL_DIR}/Src/")

set(STM32F407_HAL_INC 
    Drivers/CMSIS/Include
    Drivers/CMSIS/Device/ST/STM32F4xx/Include
    Drivers/STM32F4xx_HAL_Driver/Inc
    Drivers/STM32F4xx_HAL_Driver/Inc/Legacy
)