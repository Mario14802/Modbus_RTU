/*
 * Modbus_Slave.c
 *
 *  Created on: Jan 18, 2025
 *      Author: MARIO
 */
#include "Modbus_Slave.h"
#include "Modbus_Slave.h"

/*
 Revision History:
 V1.1 : Higher response on case of 0ms silent interval, message is prepared and sent immediately
 Aded CMD 0x17 for write and read at the  same time

 */

void MB_Slave_ProcessMessage(MB_Slave_t *MB);

// Initialize the modbus
// IPs: Modbus instance with all parameters
MB_Status_t MB_Slave_Init(MB_Slave_t *MB)
{
	MB->IsInitialized = 0;

	// allocate the TX and RX Buffers
	MB->TxBuffer = malloc(MB->TX_RX_Buffer_Size);
	MB->RxBuffer = malloc(MB->TX_RX_Buffer_Size);

	// allocate the MODBUS memory area to be accessed by the master
	MB->HoldingRegs = malloc(MB->HoldingRegSize * sizeof(uint16_t));
	MB->InputRegs = malloc(MB->InputRegSize * sizeof(uint16_t));

	memset(MB->HoldingRegs, 0, MB->HoldingRegSize * sizeof(uint16_t));
	memset(MB->InputRegs, 0, MB->InputRegSize * sizeof(uint16_t));

	MB->InputBits = malloc(sizeof(MB->InputBitsSize));
	MB->CoilBits = malloc(sizeof(MB->CoilBitsSize));

	memset(MB->CoilBits, 0, MB->CoilBitsSize * sizeof(uint8_t));
	memset(MB->InputBits, 0, MB->InputBitsSize * sizeof(uint8_t));

	if (MB->SLA == 0)
	{
		return MB_INIT_ERR_INVALID_SLA;
	}

	// check the buffers
	if (!MB->TxBuffer || !MB->RxBuffer || !MB->HoldingRegs || !MB->InputRegs || !MB->CoilBits || !MB->InputBits)
	{
		return MB_INIT_ERR_MEM_ALLOC;
	}

	// check the function pointers to be called by the MB library
	else if (!MB->hw_interface.MB_Activate_TX || !MB->hw_interface.MB_StartListening || !MB->hw_interface.MB_Transmit || !MB->hw_interface.MB_Request_Recieved)
	{
		return MB_INIT_ERR_FN_PTR;
	}

	MB->IsInitialized = 1;
	MB->Seq = 0;
	MB->MB_Stat = MB_INIT_OK;
	return MB->MB_Stat;
}

// to be called periodicaly to check for incoming messages
void MB_Slave_Routine(MB_Slave_t *MB, uint32_t Ticks)
{
	if (!MB->IsInitialized)
	{
		return;
	}
	switch (MB->Seq)
	{
		// start the receiving sequence
		case 0:
			MB->TX_MSG_LEN = 0;
			MB->RX_LEN = 0;
			MB->hw_interface.MB_Activate_TX(0);
			MB->hw_interface.MB_StartListening();
			MB->Seq = 5;
			break;

			// start of the potential frame
		case 5:
			if (MB->RX_LEN > 0)
			{
				MB->Ticks = Ticks + MB->RX_Timeout;
				MB->Seq = 10;
			}
			break;
			// waiting for the first 6 bytes in the message
			// SLA, FCN, reg address, and num of registers
		case 10:
			// check for timeout
			if (Ticks > MB->Ticks)
			{
				MB->RX_LEN = 0;
				MB->Seq = 5;
			}
			else if ((MB->RX_LEN > 6 && (MB->RxBuffer[1] != MB_PRST_READ_MUL_REGS)) || (MB->RX_LEN > 10 && (MB->RxBuffer[1] == MB_PRST_READ_MUL_REGS)))
			{
				MB->RX_SLA = MB->RxBuffer[0];
				MB->Fcn = MB->RxBuffer[1];

				// get the reg address
				MB->Reg_Address = (MB->RxBuffer[2] << 8) | MB->RxBuffer[3];

				// get the num of registers
				MB->NumOfRegisters = (MB->RxBuffer[4] << 8) | MB->RxBuffer[5];

				// compute the length of the incoming message
				switch (MB->Fcn)
				{
					case MB_READ_COIL:
					case MB_READ_INPUT_STAT:
						MB->RX_MSG_LEN = 8;
						// get the TX length
						MB->TX_MSG_LEN = 5 + (MB->NumOfRegisters / 8) + (MB->NumOfRegisters % 8 ? 1 : 0);
						MB->Target_MEM_Start = MB->Fcn == MB_READ_COIL ? MB->CoilBitsStart : MB->InputBitsStart;
						MB->Target_MEM_End = MB->Target_MEM_Start + (MB->Fcn == MB_READ_COIL ? MB->CoilBitsSize * 8 : MB->InputBitsSize * 8);
						break;

					case MB_READ_MUL_HLD_REG:
					case MB_READ_INPUT_REG:
						MB->RX_MSG_LEN = 8;
						MB->TX_MSG_LEN = 5 + (MB->NumOfRegisters * 2);
						MB->Target_MEM_Start = MB->Fcn == MB_READ_MUL_HLD_REG ? MB->HoldingRegStart : MB->InputRegsStart;
						MB->Target_MEM_End = MB->Target_MEM_Start + (MB->Fcn == MB_READ_MUL_HLD_REG ? MB->HoldingRegSize : MB->InputRegSize);
						break;

					case MB_FRC_SNG_COIL:
					case MB_PRST_SNG_REG:
						MB->NumOfRegisters = 1;
						MB->RX_MSG_LEN = 8;
						MB->TX_MSG_LEN = 8;
						MB->Target_MEM_Start = MB->Fcn == MB_FRC_SNG_COIL ? MB->CoilBitsStart : MB->HoldingRegStart;
						MB->Target_MEM_End = MB->Target_MEM_Start + (MB->Fcn == MB_FRC_SNG_COIL ? MB->CoilBitsSize * 8 : MB->HoldingRegSize);
						break;

					case MB_FRC_MUL_COILS:
						MB->RX_MSG_LEN = 9 + (MB->NumOfRegisters / 8) + (MB->NumOfRegisters % 8 ? 1 : 0);
						MB->TX_MSG_LEN = 8;
						MB->Target_MEM_Start = MB->CoilBitsStart;
						MB->Target_MEM_End = MB->Target_MEM_Start + (MB->CoilBitsSize * 8);
						break;

					case MB_PRST_MUL_REGS:
						MB->RX_MSG_LEN = 9 + (MB->NumOfRegisters * 2);
						MB->TX_MSG_LEN = 8;
						MB->Target_MEM_Start = MB->HoldingRegStart;
						MB->Target_MEM_End = MB->Target_MEM_Start + (MB->HoldingRegSize);
						break;

					case MB_PRST_READ_MUL_REGS:
						// get the number of write registers
						MB->Reg_Address_W = (MB->RxBuffer[6] << 8) | MB->RxBuffer[7];
						MB->NumOfW_Regs = (MB->RxBuffer[8] << 8) | MB->RxBuffer[9];
						MB->ByteCount = MB->RxBuffer[10];
						MB->RX_MSG_LEN = 13 + (2 * MB->NumOfW_Regs);
						MB->TX_MSG_LEN = 5 + (2 * MB->NumOfRegisters);
						MB->Target_MEM_Start = MB->HoldingRegStart;
						MB->Target_MEM_End = MB->Target_MEM_Start + (MB->HoldingRegSize);
						break;
				}

				// in case the whole message was received, then proceed to decoding immediately
				if (MB->RX_LEN >= MB->RX_MSG_LEN)
				{
					MB->MB_Stat = MB_STAT_OK;
					MB_Slave_ProcessMessage(MB);
					if (MB->RX_Silent_Interval_MS == 0)
					{
						MB->hw_interface.MB_Activate_TX(1); // activate transmitter
						MB->hw_interface.MB_Transmit(MB->TxBuffer, MB->TX_MSG_LEN);
						// disable TX mode in case of autocomplete (non blocking call)
						if (MB->TX_Automplete)
						{
							MB->hw_interface.MB_Activate_TX(0);
						}
						MB->Seq = 40;
					}
					else
					{
						// load the delay before transmitting
						MB->Ticks = Ticks + MB->RX_Silent_Interval_MS;
						MB->Seq = 30;
					}
				}
				else
				{
					// jump to the next step
					MB->Seq = 20;
				}
			}
			break;

			// wait for the rest of the message
		case 20:
			if (MB->RX_LEN >= MB->RX_MSG_LEN)
			{
				MB->MB_Stat = MB_STAT_OK;
				MB_Slave_ProcessMessage(MB);
				// for a zero wait state interfae (like modbus over USB)
				if (MB->RX_Silent_Interval_MS == 0)
				{
					MB->hw_interface.MB_Activate_TX(1); // activate transmitter
					MB->hw_interface.MB_Transmit(MB->TxBuffer, MB->TX_MSG_LEN);
					// disable TX mode in case of autocomplete (non blocking call)
					if (MB->TX_Automplete)
					{
						MB->hw_interface.MB_Activate_TX(0);
					}
					MB->Seq = 40;
				}
				else
				{
					MB->Ticks = Ticks + MB->RX_Silent_Interval_MS;
					MB->Seq = 30;
				}
			}
			//in case of timeoout
			else if (Ticks > MB->Ticks)
			{
				MB->MB_Stat = MB_RX_TIMEOUT;
				MB->hw_interface.MB_Request_Recieved(MB);
				MB->Seq = 0;
				return;
			}
			break;

			// transmit the reply if needed
		case 30:
			if (MB->MB_Stat == MB_STAT_OK)
			{
				// wait for silent interval to finish
				if (Ticks > MB->Ticks)
				{
					MB->hw_interface.MB_Activate_TX(1); // activate transmitter
					MB->hw_interface.MB_Transmit(MB->TxBuffer, MB->TX_MSG_LEN);
					// disable TX mode in case of autocomplete (non blocking call)
					if (MB->TX_Automplete)
					{
						MB->hw_interface.MB_Activate_TX(0);
					}
					MB->Seq = 40;
				}
			}
			else
			{
				MB->Seq = 0;
			}
			break;

			// wait till tx is complete
		case 40:
			if (MB->TX_Complete || MB->TX_Automplete)
			{
				MB->MB_Stat = MB_STAT_OK;
				MB->Seq = 0;
			}
			break;

		default:
			MB->Seq = 0;
			break;
	}
}

// transmits the data after doing the CRC calculation
/*MB_Status_t MB_Slave_Transmit_Data(MB_Slave_t *MB, uint16_t Len) {
 MB->TX_Complete = 0;
 //compute the CRC
 uint16_t MB_CRC = crc16(MB->TxBuffer, Len);
 MB->TX_LEN = Len + 2;
 MB->TxBuffer[Len] = (uint8_t) MB_CRC;
 MB->TxBuffer[Len + 1] = (uint8_t) (MB_CRC >> 8);
 if (MB->hw_interface.MB_Transmit == 0) {
 return MB_TX_ERR_Invalid_PTR;
 }
 MB->hw_interface.MB_Transmit(MB->TxBuffer, MB->TX_LEN);
 return MB_OK;
 }*/

// These functions are called from the user program
// used to signal the library that the TX operation s complete
void MB_Slave_TX_Complete(MB_Slave_t *MB)
{
	MB->TX_Complete = 1;
	MB->hw_interface.MB_Activate_TX(0);
}

// used to add single bytes to the RX buffer
MB_Status_t MB_Slave_Add_Byte(MB_Slave_t *MB, uint8_t data)
{
	// in case of RX buffer overflow
	if (MB->RX_LEN >= MB->TX_RX_Buffer_Size)
	{
		MB->RX_LEN = 0;
		return MB_RX_OVF;
	}
	else
	{
		MB->RxBuffer[MB->RX_LEN++] = data;
		return MB_STAT_OK;
	}
}

/// @brief (internal only) prepares an exception reply to be issed to the master
void MB_Slave_Prepare_Exception(MB_Slave_t *MB, MB_EXC_CODE C)
{
	MB->TX_MSG_LEN = 5;
	MB->TxBuffer[0] = MB->RxBuffer[0];
	MB->TxBuffer[1] = MB->RxBuffer[1] | 0x80;
	MB->TxBuffer[2] = C;
}

/// @fn void MB_Slave_ProcessMessage(MB_Slave_t*)
/// @brief internal api function
/// this is where the message is processed,
/// @param MB ptr to the Modbus slave struct
void MB_Slave_ProcessMessage(MB_Slave_t *MB)
{
	// return MB_OK;
	// check the message integrity
	uint16_t crc = crc16(MB->RxBuffer, MB->RX_LEN - 2);
	uint16_t crc_data = MB->RxBuffer[MB->RX_LEN - 1] | (MB->RxBuffer[MB->RX_LEN - 2] << 8);

	// check the CRC
	if (crc != crc_data)
	{
		// return CRC error
		MB->MB_Stat = MB_RX_ERR_CRC;
		return;
	}
	else if (MB->SLA != MB->RX_SLA)
	{
		MB->MB_Stat = MB_RX_ERR_ADD;
		return;
	}

	// used for internal operations
	uint16_t Off = 0;
	uint16_t *ptr;

	// check for a valid address and data length
	if (MB->Target_MEM_Start > MB->Reg_Address || (MB->Reg_Address + MB->NumOfRegisters) > MB->Target_MEM_End)
	{
		MB_Slave_Prepare_Exception(MB, MB_ILLEGAL_DATA_ADDRESS);
	}
	else
	{
		// get the function code
		switch (MB->Fcn)
		{
			case MB_READ_COIL:
			case MB_READ_INPUT_STAT:
				// callback is issued before preparing the reply
				MB->hw_interface.MB_Request_Recieved(MB);
				// encode the bits to be acquirted into the message
				MB_Encode_Coils(MB->Fcn == MB_READ_COIL ? MB->CoilBits : MB->InputBits, MB->Reg_Address, MB->TxBuffer + 3, MB->NumOfRegisters);
				MB->TxBuffer[2] = (MB->NumOfRegisters / 8) + (MB->NumOfRegisters % 8 ? 1 : 0);
				memcpy(MB->TxBuffer, MB->RxBuffer, 2);
				break;

				// for read multiple holding registers.
			case MB_READ_MUL_HLD_REG:
			case MB_READ_INPUT_REG:
				// callback is issued before preparing the reply
				MB->hw_interface.MB_Request_Recieved(MB);
				ptr = MB->Fcn == MB_READ_INPUT_REG ? MB->InputRegs : MB->HoldingRegs;
				// encode the registers
				for (uint16_t x = 0; x < MB->NumOfRegisters; x++)
				{
					MB_Encode_UInt16(MB->TxBuffer + 3, ptr[x + MB->Reg_Address], &Off);
				}

				memcpy(MB->TxBuffer, MB->RxBuffer, 2);
				// compute and load the byte count
				MB->TxBuffer[2] = 2 * MB->NumOfRegisters;
				break;

			case MB_FRC_SNG_COIL:
			case MB_PRST_SNG_REG:
				if (MB->Fcn == MB_FRC_SNG_COIL)
				{
					MB_Parse_Coils(MB->RxBuffer + 4, MB->Reg_Address, MB->CoilBits, 1);
				}
				else
				{
					MB_Parse_UInt16(MB->RxBuffer + 4, &Off, &MB->HoldingRegs[MB->Reg_Address]);
				}
				// callback is issued after parsing the data
				MB->hw_interface.MB_Request_Recieved(MB);
				// copy the slave address, function code, reg address, numofregisters
				// to the TX Buffer
				memcpy(MB->TxBuffer, MB->RxBuffer, 6);
				break;

				// num of registers here indicate the num of coil bits
			case MB_FRC_MUL_COILS:
			case MB_PRST_MUL_REGS:
				if (MB->Fcn == MB_FRC_MUL_COILS)
				{
					MB_Parse_Coils(MB->RxBuffer + 7, MB->Reg_Address, MB->CoilBits, MB->NumOfRegisters);
				}
				else
				{
					for (uint16_t x = 0; x < MB->NumOfRegisters; x++)
					{
						MB_Parse_UInt16(MB->RxBuffer + 7, &Off, &MB->HoldingRegs[x + MB->Reg_Address]);
					}
				}
				// callback is issued after parsing the data
				MB->hw_interface.MB_Request_Recieved(MB);
				// copy the slave address, function code, reg address, numofregisters
				// to the TX Buffer
				memcpy(MB->TxBuffer, MB->RxBuffer, 6);
				break;

			case MB_PRST_READ_MUL_REGS:
				// preset the requested holding registers (the write part)
				for (uint16_t x = 0; x < MB->NumOfW_Regs; x++)
				{
					MB_Parse_UInt16(MB->RxBuffer + 11, &Off, &MB->HoldingRegs[x + MB->Reg_Address_W]);
				}
				// issue the callback before loading the data intot he buffer
				MB->hw_interface.MB_Request_Recieved(MB);
				ptr = MB->HoldingRegs;
				// encode the registers (the read part)
				Off = 0;
				for (uint16_t x = 0; x < MB->NumOfRegisters; x++)
				{
					MB_Encode_UInt16(MB->TxBuffer + 3, ptr[x + MB->Reg_Address], &Off);
				}
				// load the byte count
				MB->TxBuffer[2] = 2 * MB->NumOfRegisters;
				// copy the slave address and functioncode from the incoming message
				memcpy(MB->TxBuffer, MB->RxBuffer, 2);
				break;

				// for illegal function code
			default:
				MB_Slave_Prepare_Exception(MB, MB_ILLEGAL_FUNC);
				break;
		}
	}
	MB->RX_LEN = 0;
	// compute the checksum
	crc = crc16(MB->TxBuffer, MB->TX_MSG_LEN - 2); // -2 to remove the CRC slots from the CRC calculation
	MB->TxBuffer[MB->TX_MSG_LEN - 2] = (uint8_t) (crc >> 8);
	MB->TxBuffer[MB->TX_MSG_LEN - 1] = (uint8_t) (crc);
	// ready to transmit
	MB->TX_Complete = 0;
	MB->MB_Stat = MB_STAT_OK;
}
