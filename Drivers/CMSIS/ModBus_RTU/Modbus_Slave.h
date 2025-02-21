/*
 * Modbus_Slave.h
 *
 *  Created on: Jan 18, 2025
 *      Author: MARIO
 */
#ifndef _MODBUS_SLAVE
#define _MODBUS_SLAVE


#ifdef __cplusplus
extern "C"
{
#endif

#define MB_SLAVE_VERSION 1.1

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "Modbus_MISC.h"

	typedef struct
	{
		// Slave address to respond to
		uint8_t SLA;

		// buffer definition
		uint8_t *TxBuffer;
		uint8_t *RxBuffer;

		uint16_t TX_RX_Buffer_Size;

		// starting address of the holding registers
		uint16_t HoldingRegStart;
		/// holds the length of the holding registers in words (the number of registers used)
		uint16_t HoldingRegSize;
		/// ptr to the holding registers
		uint16_t *HoldingRegs;

		// starting address of the input registers
		uint16_t InputRegsStart;
		/// holds the length of the input registers in words
		uint16_t InputRegSize;
		/// ptr to the Input Registers
		uint16_t *InputRegs;

		// starting address of input bits
		uint16_t InputBitsStart;
		/// holds the length of the Input bits grouped as bytes
		uint16_t InputBitsSize;
		/// ptr to the Input bits byte
		uint8_t *InputBits;

		// starting address of coil bits
		uint16_t CoilBitsStart;
		/// holds the length of the Coil bits grouped as bytes
		uint16_t CoilBitsSize;
		/// ptr to the coil bits byte
		uint8_t *CoilBits;
		// starting address (Address oFfset)
		uint16_t CoilBitsStartAdd;

		/// length of the received message
		uint16_t RX_LEN;

		/// time after which, the reply is transmitted
		uint16_t RX_Silent_Interval_MS;
		uint16_t RX_Timeout;

		// holds the HI interface functions as function pointers
		struct MB_Slave_Iface_t
		{
			// used to call the HW start transmit function
			MB_Status_t (*MB_Transmit)(uint8_t *data, uint16_t len);
			// used for the MB to call the HW start listening function if needed
			MB_Status_t (*MB_StartListening)();
			// used to control the HW IO that controls the direction of transmission
			MB_Status_t (*MB_Activate_TX)(uint8_t TX_ON);

			/// @brief called by the library once a message is recieved
			MB_Status_t (*MB_Request_Recieved)(void *ptr);
		} hw_interface;

		// internal variables used for the purpose of operation
		uint8_t TX_Automplete : 1; // raises the TX complete flag once the function exits
		uint8_t IsInitialized : 1;
		uint8_t TX_Complete : 1;

		uint32_t Ticks;

		// variabels for processing requests
		uint16_t TX_MSG_LEN;
		uint16_t RX_MSG_LEN;	 // the exoected message length is computed and stpred inside her
		uint8_t RX_SLA;			 // the slave address of the incoming messager
		uint16_t Reg_Address;	 // the 16-bit reg address from the decoded message
		uint16_t Reg_Address_W;	 // write register address used in combined Read Write mode (0x17)
		uint16_t NumOfRegisters; // the number of 16-bit registers inside the message
		uint16_t NumOfW_Regs;	 // used to get the number of write registers in the combined RW commmand (0x17)
		uint8_t ByteCount;
		MB_FC_t Fcn;			   // the function code inside the message
		uint8_t Seq;			   // internal variable for the state machine
		uint16_t Target_MEM_Start; // starting address of the targeted memory region
		uint16_t Target_MEM_End;   // size of the target memory in memory units (bits for coils and words for registers)
		MB_Status_t MB_Stat;

	} MB_Slave_t;

	/*
	how to use the library:
	1. the user defines a new MB_Slave_t instance and sets the appropriate configuration
		depending on the requirement
	2. the user defines 4 functions to be linked to the HW_Interface struct fcn pointers
		in the salve handle
	3. the user calles the MB_Slave_Routine in the infinite loop
	4. when a request is recieved, the library will read the data inside the request
		then issues a clllback to the user
	5. once the callbackis returned, the library prepares the reply and send it
		back to the master device
	 */

	/*                   MODBUS Slave Functions                   */

	/// @brief Initializes a modbus slave instance (to be called before using the library)
	/// @param MB Ptr to the modbus handle
	/// @return MB_INIT_OK in case of success
	MB_Status_t MB_Slave_Init(MB_Slave_t *MB);

	/// @fn void MB_Master_Routine(MB_Master_t*, uint32_t)
	/// @brief
	/// Routine call where the the master handles the necessary functions
	/// @param MB ptr to the MB_Master_t struct
	/// @param Ticks the current system tick count, like millis() for arduinovoid MB_Slave_Routine(MB_Slave_t *MB, uint32_t Ticks);
	void MB_Slave_TX_Complete(MB_Slave_t *MB);

	/// @fn MB_Status_t MB_Master_Add_Byte(MB_Master_t*, uint8_t)
	/// @brief
	/// add a byte inside the receive buffer for the MB master
	/// @param MB ptr to the MB_Master_t struct holding its configuration
	/// @param data byte to be added
	/// @return
	MB_Status_t MB_Slave_Add_Byte(MB_Slave_t *MB, uint8_t data);

	/// @brief Slave routine to be called periodically
	/// @param MB
	/// @param Ticks
	void MB_Slave_Routine(MB_Slave_t *MB, uint32_t Ticks);

#ifdef __cplusplus
}
#endif

#endif
