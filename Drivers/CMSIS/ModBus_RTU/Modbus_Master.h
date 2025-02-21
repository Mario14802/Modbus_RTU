/*
 * Modbus_Master.h
 *
 *  Created on: Jan 18, 2025
 *      Author: MARIO
 */

#ifndef _MOSBUS_MASTER_H
#define _MOSBUS_MASTER_H

#ifdef __cplusplus
extern "C"
{
#endif

#define MB_MASTER_VERSION 1.1

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "Modbus_MISC.h"
#include <stdbool.h>

	/// @struct
	/// @brief
	/// the master MB controller main struct
	typedef struct
	{
		// buffer definition
		uint8_t *TxBuffer;
		uint8_t *RxBuffer;

		// lengths of the buffers
		uint16_t TX_RX_BufferSize;

		// ptr to where the data field in the TX starts
		uint8_t *Ptr_TX_Data;

		// used to track down the length of the data field variables to be transmitted
		uint16_t TX_Data_Offset;

		// holders for the length of the received data length
		uint16_t RX_LEN;

		// time after which, the reply is transmitted
		uint16_t RX_Timeout;

		uint8_t SLA;

		void *ptr; // pointer to the data passed by the user

		/// @struct MB_Master_Iface_t
		/// @brief
		/// contains all the necessary function pointers needed for the modbus library to correctly operate
		struct MB_Master_Iface_t
		{

			/// @fn MB_Status_t (*MB_Transmit)(uint8_t*, uint16_t)
			/// @brief
			/// used to allow the MBlibrary to transmit data
			/// the user needs to implement the function that talks to the HW using the specific MCAL
			/// @param ptr to the 8-bit unsigned array of data to be transmitted
			/// @param len length of the message
			/// @return returns MB_OK in case of success (MB_OK needs to be implemented inside the user coed)
			MB_Status_t (*MB_Transmit)(uint8_t *data, uint16_t len);

			/// @fn MB_Status_t (*MB_StartListening)()
			/// @brief
			/// used by the library to activate the receiver, or either to flush the RX buffer to do a fresh capture of data
			/// the user needs to do the implementation
			/// @return
			MB_Status_t (*MB_StartListening)();

			/// @fn MB_Status_t (*MB_Activate_TX)()
			/// @brief
			/// used to activate the transmit mode
			/// user needs to implement a function that controls the TX/RX GPIO, 1 for TX mode, and 0 for RX Mode
			/// @param TX_ON 1 for TX and 0 for RX
			/// @return
			MB_Status_t (*MB_Activate_TX)(uint8_t TX_ON);

			/// @brief called by the library to inform the user about
			/// a finished data transaction
			MB_Status_t (*MB_Request_Recieved)(void *ptr);
		} hw_interface;

		// internal variables used for the purpose of operation
		uint8_t TX_Automplete : 1; // raises the TX complete flag once the function exits
		uint8_t IsInitialized : 1;
		uint8_t TX_Complete : 1;
		uint8_t Busy : 1;

		// used internally to generate non blocking delays
		uint32_t Ticks;

		uint16_t TX_MSG_LEN;
		uint16_t RX_MSG_LEN;	 // the expected message length is computed and stpred inside her
		uint16_t Reg_Address;	 // the 16-bit reg address from the decoded message
		uint16_t Reg_Address_W;	 // write register address used in combined Read Write mode (0x17)
		uint16_t NumOfRegisters; // the number of registers inside the message
		uint16_t NumOfW_Regs;	 // used to get the number of write registers in the combined RW commmand (0x17)
		uint8_t RX_ByteCount;	 // byte count in the RX frame
		MB_FC_t Fcn;			 // the function code inside the message
		uint8_t Seq;			 // internal variable for the state machine

		MB_Status_t MB_Stat;	 //holds the status of the transaction
		MB_EXC_CODE MB_EXE;      //holds the exception code for the modbus
	} MB_Master_t;

	/*
	how to use the library:
	1. the user defines a new MB_Master_t instance and sets the appropriate configuration
		depending on the requirement
	2. the user defines 4 functions to be linked to the HW_Interface struct fcn pointers
		in the master handle
	3. the user calles the MB_Slave_Routine in the infinite loop
	4. the user then select an opertion to perform (write coil, read holding register, etc...)
	5. once the reply is recoeved, a calback will be issued back to the application
	 */

	/// @fn MB_Status_t MB_Master_Init(MB_Master_t*)
	/// @brief
	/// Initializes the MB master mode
	/// @param MB ptr to the MB_Master_t struct holding its configuration
	/// @return
	MB_Status_t MB_Master_Init(MB_Master_t *MB);

	/// @fn void MB_Master_Routine(MB_Master_t*, uint32_t)
	/// @brief
	/// Routine call where the the master handles the necessary functions
	/// @param MB ptr to the MB_Master_t struct
	/// @param Ticks the current system tick count, like millis() for arduino
	void MB_Master_Routine(MB_Master_t *MB, uint32_t Ticks);

	/// @fn void MB_Master_TX_Complete(MB_Master_t*)
	/// @brief
	/// sets the TXCPLT flag for the master MB
	/// @param MB ptr to the MB_Master_t struct holding its configuration
	void MB_Master_TX_Complete(MB_Master_t *MB);

	/// @fn MB_Status_t MB_Master_Add_Byte(MB_Master_t*, uint8_t)
	/// @brief
	/// add a byte inside the receive buffer for the MB master
	/// @param MB ptr to the MB_Master_t struct holding its configuration
	/// @param data byte to be added
	/// @return
	MB_Status_t MB_Master_Add_Byte(MB_Master_t *MB, uint8_t data);

	/// @fn MB_Status_t MB_Master_Send_Request(MB_Master_t*, uint8_t, uint16_t, uin16_t)
	/// @brief
	///	Sends the modbus request to the selected slave
	/// in case of FCN 0x17, the user needs to set the NumOfW_Regs and
	/// the Reg_Address_W manually
	/// @param MB: Ptr to the modbus master struct
	/// @param SLA : Slave Address
	/// @param Reg_Address: modbus register address field (16-bit)
	/// @param Len : the length field in the MB frame
	/// @return
	MB_Status_t MB_Master_Send_Request(MB_Master_t *MB, uint8_t SLA,
									   uint16_t Reg_Address, uint16_t Len);

	/// @brief Reads holding registers starting from \p RegAddress for \p LEN words from Modbus slave at \p SLA
	/// @param MB Ptr to the master handle
	/// @param SLA Salve Address
	/// @param RegAddress Address of the register
	/// @param Len numbert of words to be read
	/// @param Data ptr to the data array to be filled
	/// @return MB_STAT_OK
	MB_Status_t MB_Master_ReadHoldingRegs(MB_Master_t *MB, uint8_t SLA, uint16_t RegAddress, uint16_t Len, uint16_t *Data);

	/// @brief Reads input registers starting from \p RegAddress for \p LEN words from Modbus slave at \p SLA
	/// @param MB Ptr to the master handle
	/// @param SLA Salve Address
	/// @param RegAddress Address of the register
	/// @param Len numbert of words to be read
	/// @param Data ptr to the data array to be filled
	/// @return MB_STAT_OK
	MB_Status_t MB_Master_ReadInputRegs(MB_Master_t *MB, uint8_t SLA, uint16_t RegAddress, uint16_t Len, uint16_t *Data);

	/// @brief Writes a singel holding register at address \p RegAddress
	/// @param MB Ptr to the master handle
	/// @param SLA Salve Address
	/// @param RegAddress Address of the register
	/// @param Data data word to be written to this address
	/// @return MB_STAT_OK
	MB_Status_t MB_Master_WriteHoldingReg(MB_Master_t *MB, uint8_t SLA, uint16_t RegAddress, uint16_t Data);

	/// @brief Writes multiple holding regsstarting at \p RegAddress for \p Len words
	/// @param MB Ptr to the master handle
	/// @param SLA Salve Address
	/// @param RegAddress Address of the register
	/// @param Len number of words to be written
	/// @param Data data array of words to be written
	/// @return MB_STAT_OK
	MB_Status_t MB_Master_WriteHoldingRegs(MB_Master_t *MB, uint8_t SLA, uint16_t RegAddress, uint16_t Len, uint16_t *Data);

	/// @brief reads the status of \p Len coils starting from address \p RegAddress
	/// @param MB Ptr to the master handle
	/// @param SLA Salve Address
	/// @param RegAddress Address of the register
	/// @param Len length of coil bits to be read
	/// @param Coils pointer to a byte array where the read data will be saved
	/// @return MB_STAT_OK
	MB_Status_t MB_Master_ReadCoils(MB_Master_t *MB, uint8_t SLA, uint16_t RegAddress, uint16_t Len, uint8_t *Coils);

	/// @brief reads the status of \p Len input bits starting from address \p RegAddress
	/// @param MB Ptr to the master handle
	/// @param SLA Salve Address
	/// @param RegAddress Address of the register
	/// @param Len length of input bits to be read
	/// @param Coils pointer to a byte array where the read data will be saved
	/// @return MB_STAT_OK
	MB_Status_t MB_Master_ReadInputs(MB_Master_t *MB, uint8_t SLA, uint16_t RegAddress, uint16_t Len, uint8_t *Inputs);

	/// @brief Writes the status of a singel coil
	/// @param MB Ptr to the master handle
	/// @param SLA Salve Address
	/// @param RegAddress Address of the register
	/// @param Coil Status of the coil 1 for HIGH and 0 for LOW
	/// @return MB_STAT_OK
	MB_Status_t MB_Master_WriteCoil(MB_Master_t *MB, uint8_t SLA, uint16_t RegAddress, uint8_t Coil);

	/// @brief Writes the status of multiple coils starting from \p RegAddress for \p Len coils
	/// @param MB Ptr to the master handle
	/// @param SLA Salve Address
	/// @param RegAddress Address of the register
	/// @param Len number of coils to be written
	/// @param Coils byte array containing the coil status to be written
	/// @return MB_STAT_OK
	MB_Status_t MB_Master_WriteCoils(MB_Master_t *MB, uint8_t SLA, uint16_t RegAddress, uint16_t Len, uint8_t *Coils);

	/// @brief Performs a combined read/write operation
	/// @param MB Ptr to the master handle
	/// @param SLA Salve Address
	/// @param RegAddress Address of the register
	/// @param ReadLen length of data to be read
	/// @param ReadData word array to hold the read data once recoeved form the slave
	/// @param WriteRegAddress starting address of the register to be written
	/// @param WriteLen number of registers to be written
	/// @param WriteData word array contining the values to be written
	/// @return MB_STAT_OK
	MB_Status_t MB_Master_ReadWriteHoldingRegs(MB_Master_t *MB,
											   uint8_t SLA,
											   uint16_t ReadRegAddress,
											   uint16_t ReadLen,
											   uint16_t *ReadData,
											   uint16_t WriteRegAddress,
											   uint16_t WriteLen,
											   uint16_t *WriteData);
#ifdef __cplusplus
}
#endif

#endif
