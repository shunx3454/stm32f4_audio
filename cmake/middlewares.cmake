file(GLOB_RECURSE LVGL_SRC Middlewares/lvgl/*.c)

file(GLOB MIDDLEWARES_SRC 
    Middlewares/Fatfs/*.c
    Middlewares/FreeRTOS/*.c
    Middlewares/FreeRTOS/portable/GCC/ARM_CM4F/port.c
    Middlewares/FreeRTOS/portable/MemMang/heap_4.c
    Middlewares/STM32_USB_Device_Library/Class/MSC/Src/*.c
    Middlewares/STM32_USB_Device_Library/Core/Src/*.c
    Middlewares/USB_APP/Src/*.c
    Middlewares/libfoxenflac/foxen/*.c
)

set(MIDDLEWARES_SRC ${MIDDLEWARES_SRC} ${LVGL_SRC})

set(MIDDLEWARES_INC
    Middlewares/Fatfs
    Middlewares/FreeRTOS/include
    Middlewares/FreeRTOS/portable/GCC/ARM_CM4F
    Middlewares/STM32_USB_Device_Library/Core/Inc
    Middlewares/STM32_USB_Device_Library/Class/MSC/Inc
    Middlewares/USB_APP/Inc
    Middlewares/lvgl
    Middlewares/libfoxenflac/foxen
    Middlewares/minimp3
)