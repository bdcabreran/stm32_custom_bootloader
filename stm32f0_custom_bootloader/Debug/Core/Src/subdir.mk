################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (9-2020-q2-update)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Core/Src/main.c \
../Core/Src/peripherals_init.c \
../Core/Src/stm32f0xx_hal_msp.c \
../Core/Src/stm32f0xx_it.c \
../Core/Src/syscalls.c \
../Core/Src/sysmem.c \
../Core/Src/system_stm32f0xx.c 

OBJS += \
./Core/Src/main.o \
./Core/Src/peripherals_init.o \
./Core/Src/stm32f0xx_hal_msp.o \
./Core/Src/stm32f0xx_it.o \
./Core/Src/syscalls.o \
./Core/Src/sysmem.o \
./Core/Src/system_stm32f0xx.o 

C_DEPS += \
./Core/Src/main.d \
./Core/Src/peripherals_init.d \
./Core/Src/stm32f0xx_hal_msp.d \
./Core/Src/stm32f0xx_it.d \
./Core/Src/syscalls.d \
./Core/Src/sysmem.d \
./Core/Src/system_stm32f0xx.d 


# Each subdirectory must supply rules for building sources it contributes
Core/Src/%.o: ../Core/Src/%.c Core/Src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m0 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F030xC -c -I../Core/Inc -I../Drivers/STM32F0xx_HAL_Driver/Inc -I../Drivers/STM32F0xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F0xx/Include -I../Drivers/CMSIS/Include -I../Core/Inc/API -I../Core/Inc/led_animation -I../Core/Inc/host_comm -O3 -ffunction-sections -fdata-sections -Wall -fstack-usage -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfloat-abi=soft -mthumb -o "$@"

