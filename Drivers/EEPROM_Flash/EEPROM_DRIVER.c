/*
 * EEPROM_DRIVER.c
 *
 *  Created on: Nov 25, 2024
 *      Author: M.W
 */

#ifndef EEPROM_FLASH_EEPROM_DRIVER_C_
#define EEPROM_FLASH_EEPROM_DRIVER_C_

#include "EEPROM_DRIVER.h"

uint8_t EE_Buffer[32];

/*
 * to retrieve the processed area from the EEPROM
 * 1. Read the saved area (of type systemparms_t)
 * 2. calculate the 16-bit checksum (over the length of the systemparams_t bytes -2
 * 3. compare the computed checksum with the retreived one
 * 	  compare the lower bytes with the lower 8-bits of the 16-bit
 * 	  checksum and the higher byte with the higher 8-bit of the 16-bit checksum
 * 4. return 1 if the checksum matches and 0 otherwise
 */

void EEPROM_Process_U8(uint16_t Add, uint8_t *Value, bool R_W) {
	if (!R_W) {
		ee_write(Add, 1, Value);
	} else {
		ee_read(Add, 1, Value);
	}
}

void EEPROM_Process_U16(uint16_t Add, uint16_t *Value, bool R_W) {
	if (!R_W) {
		EE_Buffer[1] = (*Value) >> 8;
		EE_Buffer[0] = *Value;
		ee_write(Add, 2, EE_Buffer);
	} else {
		ee_read(Add, 2, EE_Buffer);
		*Value = EE_Buffer[0] | (EE_Buffer[1] << 8);
	}
}

void EEPROM_Process_U32(uint16_t Add, uint32_t *Value, bool R_W) {
	if (!R_W) {
		EE_Buffer[3] = (*Value) >> 24;
		EE_Buffer[2] = (*Value) >> 16;
		EE_Buffer[1] = (*Value) >> 8;
		EE_Buffer[0] = *Value;
		ee_write(Add, 4, EE_Buffer);
	} else {
		ee_read(Add, 4, EE_Buffer);
		*Value = EE_Buffer[0] | (EE_Buffer[1] << 8) | (EE_Buffer[2] << 16)
				| (EE_Buffer[3] << 24);
	}
}

void EEPROM_Process_Flt(uint16_t Add, float *Val, bool R_W) {
	uint32_t *Value = (uint32_t*) Val;
	if (!R_W) {
		EE_Buffer[3] = (*Value) >> 24;
		EE_Buffer[2] = (*Value) >> 16;
		EE_Buffer[1] = (*Value) >> 8;
		EE_Buffer[0] = *Value;
		ee_write(Add, 4, EE_Buffer);
	} else {
		ee_read(Add, 4, EE_Buffer);
		*Value = EE_Buffer[0] | (EE_Buffer[1] << 8) | (EE_Buffer[2] << 16)
				| (EE_Buffer[3] << 24);
	}
}

void EEPROM_Process_Area(uint16_t Add, uint8_t *Val, uint16_t Len, bool R_W) {
	if (!R_W) {
		//ee_write(Add, Len, Val);
		ee_writeToRam(Add, Len, Val);
		ee_commit();

	} else {
		ee_read(Add, Len, Val);
	}
}
#endif /* EEPROM_FLASH_EEPROM_DRIVER_C_ */
