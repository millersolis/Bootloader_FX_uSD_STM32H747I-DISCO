################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (10.3-2021.10)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
C:/Users/mille/Sandbox/Bootloader_FX_uSD_STM32H747I-DISCO/bootloader/Middlewares/ST/filex/common/drivers/fx_stm32_sd_driver.c 

OBJS += \
./Middlewares/Interfaces/File\ System/FileX\ SD\ interface/fx_stm32_sd_driver.o 

C_DEPS += \
./Middlewares/Interfaces/File\ System/FileX\ SD\ interface/fx_stm32_sd_driver.d 


# Each subdirectory must supply rules for building sources it contributes
Middlewares/Interfaces/File\ System/FileX\ SD\ interface/fx_stm32_sd_driver.o: C:/Users/mille/Sandbox/Bootloader_FX_uSD_STM32H747I-DISCO/bootloader/Middlewares/ST/filex/common/drivers/fx_stm32_sd_driver.c Middlewares/Interfaces/File\ System/FileX\ SD\ interface/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m7 -std=gnu11 -g3 -DDEBUG -DCORE_CM7 -DUSE_HAL_DRIVER -DSTM32H747xx -DTX_INCLUDE_USER_DEFINE_FILE -DFX_INCLUDE_USER_DEFINE_FILE -c -I../Core/Inc -I../../Drivers/STM32H7xx_HAL_Driver/Inc -I../../Drivers/STM32H7xx_HAL_Driver/Inc/Legacy -I../../Drivers/CMSIS/Device/ST/STM32H7xx/Include -I../../Drivers/CMSIS/Include -I../AZURE_RTOS/App -I../FileX/App -I../FileX/Target -I../../Middlewares/ST/filex/common/inc/ -I../../Middlewares/ST/filex/ports/generic/inc/ -I../../Middlewares/ST/threadx/common/inc/ -I../../Middlewares/ST/threadx/ports/cortex_m7/gnu/inc/ -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"Middlewares/Interfaces/File System/FileX SD interface/fx_stm32_sd_driver.d" -MT"$@" --specs=nano.specs -mfpu=fpv5-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Middlewares-2f-Interfaces-2f-File-20-System-2f-FileX-20-SD-20-interface

clean-Middlewares-2f-Interfaces-2f-File-20-System-2f-FileX-20-SD-20-interface:
	-$(RM) ./Middlewares/Interfaces/File\ System/FileX\ SD\ interface/fx_stm32_sd_driver.cyclo ./Middlewares/Interfaces/File\ System/FileX\ SD\ interface/fx_stm32_sd_driver.d ./Middlewares/Interfaces/File\ System/FileX\ SD\ interface/fx_stm32_sd_driver.o ./Middlewares/Interfaces/File\ System/FileX\ SD\ interface/fx_stm32_sd_driver.su

.PHONY: clean-Middlewares-2f-Interfaces-2f-File-20-System-2f-FileX-20-SD-20-interface

