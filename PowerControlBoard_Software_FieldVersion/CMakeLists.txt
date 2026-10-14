cmake_minimum_required(VERSION 3.20)

project(JointSuperCap LANGUAGES C CXX ASM)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS ON)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(FIRMWARE_NAME JointSuperCap-v1.0-debug)

file(GLOB APPLICATION_CPP CONFIGURE_DEPENDS Core/Src/*.cpp)

add_executable(firmware
    Core/Src/main.c
    Core/Src/gpio.c
    Core/Src/adc.c
    Core/Src/fdcan.c
    Core/Src/tim.c
    Core/Src/stm32g4xx_it.c
    Core/Src/stm32g4xx_hal_msp.c
    Core/Src/system_stm32g4xx.c
    Core/Src/sysmem.c
    Core/Src/syscalls.c
    Core/Src/hrtim.c
    Core/Src/dma.c
    Core/Src/comp.c
    Core/Src/opamp.c
    Core/Src/dac.c
    Drivers/STM32G4xx_HAL_Driver/Src/stm32g4xx_hal_adc.c
    Drivers/STM32G4xx_HAL_Driver/Src/stm32g4xx_hal_adc_ex.c
    Drivers/STM32G4xx_HAL_Driver/Src/stm32g4xx_ll_adc.c
    Drivers/STM32G4xx_HAL_Driver/Src/stm32g4xx_hal.c
    Drivers/STM32G4xx_HAL_Driver/Src/stm32g4xx_hal_rcc.c
    Drivers/STM32G4xx_HAL_Driver/Src/stm32g4xx_hal_rcc_ex.c
    Drivers/STM32G4xx_HAL_Driver/Src/stm32g4xx_hal_flash.c
    Drivers/STM32G4xx_HAL_Driver/Src/stm32g4xx_hal_flash_ex.c
    Drivers/STM32G4xx_HAL_Driver/Src/stm32g4xx_hal_flash_ramfunc.c
    Drivers/STM32G4xx_HAL_Driver/Src/stm32g4xx_hal_gpio.c
    Drivers/STM32G4xx_HAL_Driver/Src/stm32g4xx_hal_exti.c
    Drivers/STM32G4xx_HAL_Driver/Src/stm32g4xx_hal_dma.c
    Drivers/STM32G4xx_HAL_Driver/Src/stm32g4xx_hal_dma_ex.c
    Drivers/STM32G4xx_HAL_Driver/Src/stm32g4xx_hal_pwr.c
    Drivers/STM32G4xx_HAL_Driver/Src/stm32g4xx_hal_pwr_ex.c
    Drivers/STM32G4xx_HAL_Driver/Src/stm32g4xx_hal_cortex.c
    Drivers/STM32G4xx_HAL_Driver/Src/stm32g4xx_hal_fdcan.c
    Drivers/STM32G4xx_HAL_Driver/Src/stm32g4xx_hal_hrtim.c
    Drivers/STM32G4xx_HAL_Driver/Src/stm32g4xx_hal_tim.c
    Drivers/STM32G4xx_HAL_Driver/Src/stm32g4xx_hal_tim_ex.c
    Drivers/STM32G4xx_HAL_Driver/Src/stm32g4xx_hal_comp.c
    Drivers/STM32G4xx_HAL_Driver/Src/stm32g4xx_hal_opamp.c
    Drivers/STM32G4xx_HAL_Driver/Src/stm32g4xx_hal_opamp_ex.c
    Drivers/STM32G4xx_HAL_Driver/Src/stm32g4xx_hal_dac.c
    Drivers/STM32G4xx_HAL_Driver/Src/stm32g4xx_hal_dac_ex.c
    ${APPLICATION_CPP}
    startup_stm32g474xx.s
)

set_target_properties(firmware PROPERTIES
    OUTPUT_NAME ${FIRMWARE_NAME}
    SUFFIX .elf
)

target_include_directories(firmware PRIVATE
    Core/Inc
    Drivers/STM32G4xx_HAL_Driver/Inc
    Drivers/STM32G4xx_HAL_Driver/Inc/Legacy
    Drivers/CMSIS/Device/ST/STM32G4xx/Include
    Drivers/CMSIS/Include
)

target_compile_definitions(firmware PRIVATE USE_HAL_DRIVER STM32G474xx)

set(MCU_FLAGS
    -mcpu=cortex-m4
    -mthumb
    -mfpu=fpv4-sp-d16
    -mfloat-abi=hard
)

target_compile_options(firmware PRIVATE
    ${MCU_FLAGS}
    -fdata-sections
    -ffunction-sections
    -fstack-usage
    -specs=nano.specs
    -pipe
    -Wfatal-errors
    -Wall
    -Wextra
    -Wpedantic
    -Wshadow
    -Wdouble-promotion
    -Wlogical-op
    -Wpointer-arith
    -Wmissing-field-initializers
    -Wno-unused-parameter
    -Wno-unused-const-variable
    -fdiagnostics-color=auto
    # Keep the project's original -Ofast behavior. Several .code_in_ram
    # functions are inline and GCC rejects their shared section at -Og.
    $<$<CONFIG:Debug>:-Ofast>
    $<$<CONFIG:Debug>:-g3>
    $<$<CONFIG:Debug>:-gdwarf-4>
    $<$<CONFIG:Release>:-Ofast>
    $<$<COMPILE_LANGUAGE:CXX>:-fno-exceptions>
    $<$<COMPILE_LANGUAGE:CXX>:-fno-rtti>
    $<$<COMPILE_LANGUAGE:CXX>:-fno-threadsafe-statics>
)

target_link_options(firmware PRIVATE
    ${MCU_FLAGS}
    -specs=nano.specs
    -T${CMAKE_CURRENT_SOURCE_DIR}/stm32g474rbtx_flash.ld
    -Wl,-Map=${CMAKE_CURRENT_BINARY_DIR}/${FIRMWARE_NAME}.map,--cref
    -Wl,--gc-sections
)
target_link_libraries(firmware PRIVATE c m nosys)

add_custom_command(TARGET firmware POST_BUILD
    COMMAND ${CMAKE_OBJCOPY} -O ihex $<TARGET_FILE:firmware> ${CMAKE_CURRENT_BINARY_DIR}/${FIRMWARE_NAME}.hex
    COMMAND ${CMAKE_OBJCOPY} -O binary -S $<TARGET_FILE:firmware> ${CMAKE_CURRENT_BINARY_DIR}/${FIRMWARE_NAME}.bin
    COMMAND ${CMAKE_SIZE} $<TARGET_FILE:firmware>
    VERBATIM
)
