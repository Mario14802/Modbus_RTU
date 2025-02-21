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

typedef bool (*EE_Write)(uint16_t Address, uint8_t *data, uint8_t len);
typedef bool (*EE_Read)(uint16_t Address, uint8_t *data, uint8_t len);

///processing functions used to read and write, from and to the EEPROm
/// the R_W flag is set to 0 for write and 1 for read
void EEPROM_Process_U8(uint16_t Add, uint8_t *Value, bool R_W);
void EEPROM_Process_U16(uint16_t Add, uint16_t *Value, bool R_W);
void EEPROM_Process_U32(uint16_t Add, uint32_t *Value, bool R_W);
void EEPROM_Process_Flt(uint16_t Add, float *Val, bool R_W);
void EEPROM_Process_Area(uint16_t Add, uint8_t *Val, uint16_t Len, bool R_W);

#endif /* EEPROM_FLASH_EEPROM_DRIVER_H_ */
