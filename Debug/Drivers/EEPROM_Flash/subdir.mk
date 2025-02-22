################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (10.3-2021.10)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Drivers/EEPROM_Flash/EEPROM_DRIVER.c \
../Drivers/EEPROM_Flash/ee.c 

OBJS += \
./Drivers/EEPROM_Flash/EEPROM_DRIVER.o \
./Drivers/EEPROM_Flash/ee.o 

C_DEPS += \
./Drivers/EEPROM_Flash/EEPROM_DRIVER.d \
./Drivers/EEPROM_Flash/ee.d 


# Each subdirectory must supply rules for building sources it contributes
Drivers/EEPROM_Flash/%.o Drivers/EEPROM_Flash/%.su Drivers/EEPROM_Flash/%.cyclo: ../Drivers/EEPROM_Flash/%.c Drivers/EEPROM_Flash/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F411xE -c -I../Core/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../Drivers/CMSIS/Include -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Drivers-2f-EEPROM_Flash

clean-Drivers-2f-EEPROM_Flash:
	-$(RM) ./Drivers/EEPROM_Flash/EEPROM_DRIVER.cyclo ./Drivers/EEPROM_Flash/EEPROM_DRIVER.d ./Drivers/EEPROM_Flash/EEPROM_DRIVER.o ./Drivers/EEPROM_Flash/EEPROM_DRIVER.su ./Drivers/EEPROM_Flash/ee.cyclo ./Drivers/EEPROM_Flash/ee.d ./Drivers/EEPROM_Flash/ee.o ./Drivers/EEPROM_Flash/ee.su

.PHONY: clean-Drivers-2f-EEPROM_Flash

