################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (10.3-2021.10)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../FileX/Target/fx_stm32_sd_driver_glue.c 

OBJS += \
./FileX/Target/fx_stm32_sd_driver_glue.o 

C_DEPS += \
./FileX/Target/fx_stm32_sd_driver_glue.d 


# Each subdirectory must supply rules for building sources it contributes
FileX/Target/%.o FileX/Target/%.su FileX/Target/%.cyclo: ../FileX/Target/%.c FileX/Target/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m7 -std=gnu11 -g3 -DDEBUG -DCORE_CM7 -DUSE_HAL_DRIVER -DSTM32H747xx -DTX_INCLUDE_USER_DEFINE_FILE -DFX_INCLUDE_USER_DEFINE_FILE -c -I../Core/Inc -I../../Drivers/STM32H7xx_HAL_Driver/Inc -I../../Drivers/STM32H7xx_HAL_Driver/Inc/Legacy -I../../Drivers/CMSIS/Device/ST/STM32H7xx/Include -I../../Drivers/CMSIS/Include -I../AZURE_RTOS/App -I../FileX/App -I../FileX/Target -I../../Middlewares/ST/filex/common/inc/ -I../../Middlewares/ST/filex/ports/generic/inc/ -I../../Middlewares/ST/threadx/common/inc/ -I../../Middlewares/ST/threadx/ports/cortex_m7/gnu/inc/ -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv5-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-FileX-2f-Target

clean-FileX-2f-Target:
	-$(RM) ./FileX/Target/fx_stm32_sd_driver_glue.cyclo ./FileX/Target/fx_stm32_sd_driver_glue.d ./FileX/Target/fx_stm32_sd_driver_glue.o ./FileX/Target/fx_stm32_sd_driver_glue.su

.PHONY: clean-FileX-2f-Target

