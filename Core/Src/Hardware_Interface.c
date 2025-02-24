/*
 * Hardware_Interface.c
 *
 *  Created on: Nov 27, 2024
 *      Author: M.W
 */

#include "Hardware_Interface.h"

extern AG_HW_Interface_t HWIN;

void EE_Write_DI(uint32_t Add, uint32_t Len, uint8_t *Value) {
	ee_write(Add, Len, Value);
}

void EE_Read_DI(uint32_t Add, uint32_t Len, uint8_t *Value) {
	ee_read(Add, Len, Value);
}

void EE_Write2ram_DI(uint32_t Add, uint32_t Len, uint8_t *Data) {
	ee_writeToRam(Add, Len, Data);
}

void EE_Commit_DI() {
	ee_commit();
}

/*void EEPROM_Process_Area_HWIN(uint8_t Address, uint8_t *Data, uint8_t Length,
 bool R_W) {
 EEPROM_Process_Area(Address, Data, Length, R_W);
 }*/

void R_W_HoldingReq() {
	EEPROM_Status eeStatus;  // Local variable to hold the status

	// Set sInitSize to the current Size value on the first run.

	// Save NV variables if requested
	if (MB_Parse_Bit(MB.CoilBits, MB_Coil_Save_NV_Variables)) {
		// Compute checksum over the unique data (exclude the last 2 bytes reserved for checksum)
		uint16_t Checksum = 0;
		for (uint16_t x = 0; x < Size - StartAfterCheckSum; x++) {
			Checksum += ((uint8_t*) &Hregs->sParams)[x];
		}
		Hregs->sParams.Checksum = Checksum;
		// Write only one copy to EEPROM
		EEPROM_Process_Area(&HWIN, 0, (uint8_t*) &Hregs->sParams, Size, Write);

		// Clear the save command bit
		MB_Encode_Bit(MB.CoilBits, MB_Coil_Save_NV_Variables, 0);

		// Update status to indicate NV variables have been saved
		eeStatus = MB_COIL_NV_NOTSAVED;
	}
	// Load NV variables if requested
	else if (MB_Parse_Bit(MB.CoilBits, MB_Coil_Load_NV_Variables)) {
		// Clear the checksum error flag before reading
		MB_Encode_Bit(MB.InputBits, MB_Input_Error_Checksum, 0);

		// Read only the unique data (first copy) from EEPROM
		EEPROM_Process_Area(&HWIN, 0, (uint8_t*) &Hregs->sParams, Size, Read);

		// Compute checksum over the unique data portion
		uint16_t Checksum = 0;
		for (uint16_t x = 0; x < Size - StartAfterCheckSum; x++) {
			Checksum += ((uint8_t*) &Hregs->sParams)[x];
		}
		// Checksum=(Checksum-36) used as a test as the it worked fine with it

		// Check if the computed checksum matches the stored checksum
		if (Hregs->sParams.Checksum != Checksum) {
			MB_Encode_Bit(MB.InputBits, MB_Input_Error_Checksum, 1);
			eeStatus = MB_INPUT_ERROR_CHECKSUM;
		} else {
			eeStatus = MB_COIL_NV_NOTLOADED;
		}

		// Clear the load command bit
		MB_Encode_Bit(MB.CoilBits, MB_Coil_Load_NV_Variables, 0);
	}
	// Reset the chip if requested
	else if (MB_Parse_Bit(MB.CoilBits, MB_Coil_Reset_Chip)) {
		HAL_NVIC_SystemReset();
	}

}

