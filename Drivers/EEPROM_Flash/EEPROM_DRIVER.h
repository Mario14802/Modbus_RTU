/*
 * EEPROM_DRIVER.h
 *
 *  Created on: Nov 25, 2024
 *      Author: M.W
 */

#ifndef EEPROM_FLASH_EEPROM_DRIVER_H_
#define EEPROM_FLASH_EEPROM_DRIVER_H_

#include <stdint.h>
#include <stdbool.h>
#include "ee.h"

typedef struct HW_Interface {
	void (*EE_Write_HWIN)(uint32_t Add, uint32_t Len, uint8_t *Value);
	void (*EE_Read_HWIN)(uint32_t Add, uint32_t Len, uint8_t *Value);
	void (*EE_Write2ram_HWIN)(uint32_t Add, uint32_t Len, uint8_t *Value);
	void (*EE_Commit_HWIN)();
} AG_HW_Interface_t;

//define for read and write for EEPROM
#define Write 0
#define Read 1
//TO START,SAVING,LOADING

///processing functions used to read and write, from and to the EEPROm
/// the R_W flag is set to 0 for write and 1 for read

void EEPROM_Process_U8(AG_HW_Interface_t *HWIN,uint16_t Add, uint8_t *Value, bool R_W);
void EEPROM_Process_U16(AG_HW_Interface_t *HWIN,uint16_t Add, uint16_t *Value, bool R_W);
void EEPROM_Process_U32(AG_HW_Interface_t *HWIN,uint16_t Add, uint32_t *Value, bool R_W);
void EEPROM_Process_Flt(AG_HW_Interface_t *HWIN,uint16_t Add, float *Val, bool R_W);
void EEPROM_Process_Area(AG_HW_Interface_t *HWIN,uint16_t Add, uint8_t *Val, uint16_t Len, bool R_W);

#endif /* EEPROM_FLASH_EEPROM_DRIVER_H_ */
