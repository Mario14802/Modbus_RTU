/*
 * Hardware_Interface.h
 *
 *  Created on: Nov 27, 2024
 *      Author: M.W
 */

#ifndef HARDWARE_INTERFACE_HARDWARE_INTERFACE_H_
#define HARDWARE_INTERFACE_HARDWARE_INTERFACE_H_

#include "../../Drivers/EEPROM_Flash/EEPROM_DRIVER.h"

#define Write 0
#define Read 1

void EEPROM_Process_Area_HWIN(uint8_t Address, uint8_t *Data, uint8_t Length,
bool R_W);

#endif /* HARDWARE_INTERFACE_HARDWARE_INTERFACE_H_ */
