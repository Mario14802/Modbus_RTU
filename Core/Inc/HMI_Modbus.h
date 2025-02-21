/*
 * HMI_Modbus.h
 *
 *  Created on: Jan 23, 2025
 *      Author: MARIO
 */

#ifndef HMI_MODBUS_H_
#define HMI_MODBUS_H_



#include "main.h"
#include "stm32f4xx_hal.h"

#include <stdint.h>
#include "../../Drivers/CMSIS/ModBus_RTU/Modbus_Slave.h"
#include "Modbus_RegMap.h"

extern UART_HandleTypeDef huart1;
extern MB_Slave_t MB;

extern UART_HandleTypeDef *uart;

extern HoldingRegs_t *Hregs; //test

MB_Status_t Init_HMI(UART_HandleTypeDef *uart, uint8_t SLA);




#endif /* HMI_MODBUS_H_ */
