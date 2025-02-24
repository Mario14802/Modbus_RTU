/*
 * Hardware_Interface.h
 *
 *  Created on: Nov 27, 2024
 *      Author: M.W
 */

#ifndef HARDWARE_INTERFACE_HARDWARE_INTERFACE_H_
#define HARDWARE_INTERFACE_HARDWARE_INTERFACE_H_

#include "../../Drivers/EEPROM_Flash/EEPROM_DRIVER.h"
#include "HMI_Modbus.h"
#include "Modbus_RegMap.h"

#define Write 0
#define Read 1

#define Size 				(sizeof(SystemParams_t))
#define StartAfterCheckSum	4

typedef enum {
	MB_COIL_NV_NOTSAVED,
	MB_COIL_NV_NOTLOADED,
	MB_INPUT_ERROR_CHECKSUM
}EEPROM_Status;

void EE_Write_DI(uint32_t Add, uint32_t Len, uint8_t *Value);
void EE_Read_DI(uint32_t Add, uint32_t Len, uint8_t *Value);
void EE_Write2ram_DI(uint32_t Add, uint32_t Len, uint8_t* Data);
void EE_Commit_DI();

/*
void EEPROM_Process_Area_HWIN(uint8_t Address, uint8_t *Data, uint8_t Length,
		bool R_W);
*/

void R_W_HoldingReq();
#endif /* HARDWARE_INTERFACE_HARDWARE_INTERFACE_H_ */
