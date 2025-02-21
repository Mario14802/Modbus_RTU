################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (12.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Drivers/CMSIS/ModBus_RTU/Modbus_MISC.c \
../Drivers/CMSIS/ModBus_RTU/Modbus_Master.c \
../Drivers/CMSIS/ModBus_RTU/Modbus_Slave.c 

OBJS += \
./Drivers/CMSIS/ModBus_RTU/Modbus_MISC.o \
./Drivers/CMSIS/ModBus_RTU/Modbus_Master.o \
./Drivers/CMSIS/ModBus_RTU/Modbus_Slave.o 

C_DEPS += \
./Drivers/CMSIS/ModBus_RTU/Modbus_MISC.d \
./Drivers/CMSIS/ModBus_RTU/Modbus_Master.d \
./Drivers/CMSIS/ModBus_RTU/Modbus_Slave.d 


# Each subdirectory must supply rules for building sources it contributes
Drivers/CMSIS/ModBus_RTU/%.o Drivers/CMSIS/ModBus_RTU/%.su Drivers/CMSIS/ModBus_RTU/%.cyclo: ../Drivers/CMSIS/ModBus_RTU/%.c Drivers/CMSIS/ModBus_RTU/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F411xE -c -I../Core/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../Drivers/CMSIS/Include -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Drivers-2f-CMSIS-2f-ModBus_RTU

clean-Drivers-2f-CMSIS-2f-ModBus_RTU:
	-$(RM) ./Drivers/CMSIS/ModBus_RTU/Modbus_MISC.cyclo ./Drivers/CMSIS/ModBus_RTU/Modbus_MISC.d ./Drivers/CMSIS/ModBus_RTU/Modbus_MISC.o ./Drivers/CMSIS/ModBus_RTU/Modbus_MISC.su ./Drivers/CMSIS/ModBus_RTU/Modbus_Master.cyclo ./Drivers/CMSIS/ModBus_RTU/Modbus_Master.d ./Drivers/CMSIS/ModBus_RTU/Modbus_Master.o ./Drivers/CMSIS/ModBus_RTU/Modbus_Master.su ./Drivers/CMSIS/ModBus_RTU/Modbus_Slave.cyclo ./Drivers/CMSIS/ModBus_RTU/Modbus_Slave.d ./Drivers/CMSIS/ModBus_RTU/Modbus_Slave.o ./Drivers/CMSIS/ModBus_RTU/Modbus_Slave.su

.PHONY: clean-Drivers-2f-CMSIS-2f-ModBus_RTU

