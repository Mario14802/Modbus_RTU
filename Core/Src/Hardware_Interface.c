/*
 * Hardware_Interface.c
 *
 *  Created on: Nov 27, 2024
 *      Author: M.W
 */

#include "Hardware_Interface.h"

void EEPROM_Process_Area_HWIN(uint8_t Address, uint8_t *Data, uint8_t Length,
bool R_W) {
	EEPROM_Process_Area(Address, Data, Length, R_W);
}
