/*
 * HMI_Modbus.c
 *
 *  Created on: Jan 23, 2025
 *      Author: MARIO
 */

#include "../Inc/HMI_Modbus.h"

MB_Slave_t MB;
UART_HandleTypeDef *uart;

HoldingRegs_t *Hregs;
InputRegs_t *Iregs;

void UART1_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size) {
	MB.RX_LEN = Size;
}

void UART1_TxCpltCallback(UART_HandleTypeDef *huart) {
	MB_Slave_TX_Complete(&MB);
}

void UART1_ErrorCallback(UART_HandleTypeDef *huart) {
	MB.hw_interface.MB_StartListening();
}

// Wrapper function to transmit data over UART1
MB_Status_t MB_Transmit_Data(uint8_t *data, uint16_t Len) {
	return HAL_UART_Transmit_DMA(uart, data, Len) == HAL_OK ?
			MB_STAT_OK : MB_TX_ERR_HW;
}

// Wrapper function to switch the system to transmit mode (necessary for RS485 2-wire mode)
MB_Status_t MB_Activate_TX(uint8_t TXON) {
	// Control the RS485 TX enable pin (if applicable)
	// Not used because we use RS232
	// HAL_GPIO_WritePin(MODBUS_TXEN_GPIO_Port, MODBUS_TXEN_Pin, TXON ? GPIO_PIN_SET : GPIO_PIN_RESET);
	return MB_STAT_OK;
}

// Wrapper function used to start listening to the UART1 bus
MB_Status_t MB_StartListening() {
	// Start receiving data either until the buffer is filled or the data stream is finished (data line returns to idle)
	HAL_UARTEx_ReceiveToIdle_IT(uart, MB.RxBuffer, MB.TX_RX_Buffer_Size);
	return MB_STAT_OK;
}

// Callback function when a master sends a request (to read or write data)
// The user can implement custom logic here. After the function exits, the Modbus reply will be automatically issued.
// Callback function when a master sends a request (to read or write data)
MB_Status_t MB_Request_Recieved(void *ptr) {
	MB_Slave_t *mb = (MB_Slave_t*) ptr;



	return MB_STAT_OK;
}

// Initialize Modbus for UART1

MB_Status_t MB_Init_UART1(UART_HandleTypeDef *huart, uint8_t SLA) {
	uart = huart;

	// Register the UART1 callbacks
	HAL_UART_RegisterRxEventCallback(huart, UART1_RxEventCallback);
	HAL_UART_RegisterCallback(huart, HAL_UART_TX_COMPLETE_CB_ID,
			UART1_TxCpltCallback);
	HAL_UART_RegisterCallback(huart, HAL_UART_ERROR_CB_ID, UART1_ErrorCallback);

	// Set buffer size
	MB.TX_RX_Buffer_Size = 256; // Set the buffer size to 256 bytes

	// Set the size of holding and input registers
	MB.HoldingRegSize = HoldingRegsSize;  // size from Modbuse reg map
	MB.InputRegSize = InputRegsSize;    // size from Modbuse reg map


	// Set the size of input bits and coil bits
	MB.InputBitsSize = 2;    // 2 bytes for input bits (16 bits)
	MB.CoilBitsSize = 2;     // 2 bytes for coil bits (16 bits)

	// Assign the function pointers
	MB.hw_interface.MB_Activate_TX = &MB_Activate_TX;
	MB.hw_interface.MB_Transmit = &MB_Transmit_Data;
	MB.hw_interface.MB_StartListening = &MB_StartListening;
	MB.hw_interface.MB_Request_Recieved = &MB_Request_Recieved; // Corrected to use MB_Request_Recieved

	// Set the slave address
	MB.SLA = SLA;

	// Configure transmission and reception settings
	MB.TX_Automplete = 1; // Disable auto-complete for TX (manual control)
	MB.RX_Silent_Interval_MS = 2; // Silent interval between messages (2 ms)
	MB.RX_Timeout = 100; // Timeout for receiving messages (100 ms)



	// Switch to listen mode
	MB.hw_interface.MB_Activate_TX(0);

	// Initialize the Modbus slave
	MB_Slave_Init(&MB);


	Hregs=(HoldingRegs_t*)MB.HoldingRegs;
	Iregs=(InputRegs_t*)MB.InputRegs;

	return MB_STAT_OK;

}
