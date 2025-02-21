/*
 * Modbus_Master.c
 *
 *  Created on: Jan 18, 2025
 *      Author: MARIO
 */
#include "Modbus_Master.h"
#include "Modbus_MISC.h"

/// @brief (internal) calculates the CRC for the message and dispatches it
/// @param MB Ptr to the master handle
void MB_Master_Transmit(MB_Master_t *MB)
{
	uint16_t crc = crc16(MB->TxBuffer, MB->TX_MSG_LEN - 2);
	MB->TxBuffer[MB->TX_MSG_LEN - 2] = (uint8_t)(crc >> 8);
	MB->TxBuffer[MB->TX_MSG_LEN - 1] = (uint8_t)(crc);
	MB->hw_interface.MB_Activate_TX(1);
	MB->hw_interface.MB_Transmit(MB->TxBuffer, MB->TX_MSG_LEN);
	if (MB->TX_Automplete)
	{
		MB->hw_interface.MB_Activate_TX(0);
		MB->hw_interface.MB_StartListening();
	}
	MB->Seq = 10;
}

/// @brief (internal) used to process the incoming message
/// @param MB Ptr to the master handle
void MB_Master_ProcessMessage(MB_Master_t *MB);

/// @brief Initializes the Modbus Master handle (to be called before using any functions inside the library)
/// @param MB Ptr to the master handle
/// @return MB_INIT_OK  in case of success
MB_Status_t MB_Master_Init(MB_Master_t *MB)
{
	MB->IsInitialized = 0;

	if (MB->RX_Timeout == 0)
	{
		MB->RX_Timeout = 200;
	}

	// allocate the TX and RX Buffers
	MB->TxBuffer = malloc(MB->TX_RX_BufferSize);
	MB->RxBuffer = malloc(MB->TX_RX_BufferSize);

	// check the buffers
	if (!MB->TxBuffer || !MB->RxBuffer)
	{
		return MB_INIT_ERR_MEM_ALLOC;
	}

	// check the function pointers to be called by the MB library
	else if (!MB->hw_interface.MB_Activate_TX || !MB->hw_interface.MB_StartListening || !MB->hw_interface.MB_Transmit || !MB->hw_interface.MB_Request_Recieved)
	{
		return MB_INIT_ERR_FN_PTR;
	}

	MB->IsInitialized = 1;
	MB->Busy = 0;
	MB->Seq = 0;
	MB->MB_Stat = MB_STAT_OK;
	return MB->MB_Stat;
}

/// @brief to be called to indicatew the end of transmission
/// @param MB Ptr to the master handle
void MB_Master_TX_Complete(MB_Master_t *MB)
{
	MB->TX_Complete = 1;
	// switch back to RX
	if (!MB->TX_Automplete)
	{
		MB->hw_interface.MB_Activate_TX(0);
		MB->hw_interface.MB_StartListening();
	}
}

/// @brief to be called to add data to the recieve buffer
/// @param MB Ptr to the master handle
/// @param data byte of data to be added
/// @return MB_Stat_OK in case of success, or MB_Overflow in case of too many bytes
MB_Status_t MB_Master_Add_Byte(MB_Master_t *MB, uint8_t data)
{
	// in case of RX buffer overflow
	if (MB->RX_LEN == MB->TX_RX_BufferSize)
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

void MB_Master_Routine(MB_Master_t *MB, uint32_t Ticks)
{
	if (!MB->IsInitialized)
	{
		return;
	}

	switch (MB->Seq)
	{
	case 0:
		MB->Busy = 0;
		MB->RX_LEN = 0;
		MB->Seq = 5;
		break;

	case 5:
		break;

		// TX data transmitted, wait for completion
	case 10:
		if (MB->TX_Automplete || MB->TX_Complete)
		{
			MB->MB_EXE = MB_NO_EXEPTION;
			MB->MB_Stat = MB_RX_WAITING_MSG;
			MB->Ticks = Ticks + MB->RX_Timeout;
			MB->Seq = 20;
		}
		break;

		// check the incoming message and check for timeout
	case 20:
		// check the received message
		if (MB->RX_LEN >= MB->RX_MSG_LEN)
		{
			MB->MB_EXE = MB_NO_EXEPTION;
			MB_Master_ProcessMessage(MB);
			MB->Seq = 0;
		}
		// check for an exception response
		// try to parse it
		else if (MB->RX_LEN == MB_EXC_LEN)
		{
			// compute the CRCL
			if (crc16(MB->RxBuffer, 3) == ((uint16_t)(MB->RxBuffer[3] << 8) | (uint16_t)MB->RxBuffer[4]) &&
				MB->RxBuffer[1] & 0x80)
			{
				MB->MB_EXE = MB->RxBuffer[2];
				MB->MB_Stat = MB_EXC_RESPONSE;
				MB->hw_interface.MB_Request_Recieved(MB);
				MB->Seq = 0;
			}
		}
		else if (Ticks > MB->Ticks)
		{
			MB->MB_Stat = MB_RX_TIMEOUT;
			MB->hw_interface.MB_Request_Recieved(MB);
			MB->Seq = 0;
		}
		break;

	default:
		MB->Seq = 0;
		break;
	}
}

/// @brief processrouting to be called periodically in the infinite loop
/// @param MB Ptr to the master handle
void MB_Master_ProcessMessage(MB_Master_t *MB)
{
	// check the message integrity
	uint16_t crc = crc16(MB->RxBuffer, MB->RX_MSG_LEN - 2);
	uint16_t crc_data = MB->RxBuffer[MB->RX_MSG_LEN - 1] | (MB->RxBuffer[MB->RX_MSG_LEN - 2] << 8);
	// check the CRC
	if (crc != crc_data)
	{
		// return CRC error
		MB->MB_Stat = MB_RX_ERR_CRC;
		return;
	}
	else if (MB->TxBuffer[0] != MB->RxBuffer[0])
	{
		MB->MB_Stat = MB_RX_ERR_ADD;
		return;
	}
	else if (MB->TxBuffer[1] != MB->RxBuffer[1])
	{
		MB->MB_Stat = MB_RX_ERR_FUN;
	}
	else
	{
		MB->MB_Stat = MB_STAT_OK;
	}

	if (MB->MB_Stat == MB_STAT_OK)
	{
		// get the function code
		switch (MB->Fcn)
		{
			// load the coil data into the registers
		case MB_READ_COIL:
		case MB_READ_INPUT_STAT:
			/// TODO: revisit this part
			MB_Parse_Coils(MB->RxBuffer + 3, 0, (uint8_t *)MB->ptr, MB->NumOfRegisters);
			break;

			// for read multiple holding registers.
		case MB_READ_MUL_HLD_REG:
		case MB_READ_INPUT_REG:
		case MB_PRST_READ_MUL_REGS:
			for (uint16_t x = 0; x < MB->NumOfRegisters; x++)
			{
				((uint16_t *)MB->ptr)[x] = MB->RxBuffer[4 + (2 * x)] | (MB->RxBuffer[3 + (2 * x)] << 8);
			}
			break;

		case MB_FRC_SNG_COIL:
		case MB_PRST_SNG_REG:
		case MB_FRC_MUL_COILS:
		case MB_PRST_MUL_REGS:
			break;

		default:
			break;
		}
	}
	else
	{
	}
	MB->hw_interface.MB_Request_Recieved(MB);
}

MB_Status_t MB_Master_ReadHoldingRegs(MB_Master_t *MB, uint8_t SLA, uint16_t RegAddress, uint16_t Len, uint16_t *Data)
{
	// set the busy flag
	MB->Busy = 1;
	MB->Fcn = MB_READ_MUL_HLD_REG;
	MB->SLA = SLA;
	MB->Reg_Address = RegAddress;
	MB->NumOfRegisters = Len;
	MB->TX_MSG_LEN = 8;
	MB->RX_MSG_LEN = 5 + (2 * MB->NumOfRegisters);

	MB->TxBuffer[0] = SLA;
	MB->TxBuffer[1] = MB->Fcn;
	MB->TxBuffer[2] = (uint8_t)(MB->Reg_Address >> 8);
	MB->TxBuffer[3] = (uint8_t)(MB->Reg_Address);
	MB->TxBuffer[4] = (uint8_t)(MB->NumOfRegisters >> 8);
	MB->TxBuffer[5] = (uint8_t)(MB->NumOfRegisters);

	// assign the data ptr to the message
	MB->ptr = (void *)Data;
	MB_Master_Transmit(MB);
	return MB_STAT_OK;
}

MB_Status_t MB_Master_ReadInputRegs(MB_Master_t *MB, uint8_t SLA, uint16_t RegAddress, uint16_t Len, uint16_t *Data)
{
	// set the busy flag
	MB->Busy = 1;
	MB->Fcn = MB_READ_INPUT_REG;
	MB->SLA = SLA;
	MB->Reg_Address = RegAddress;
	MB->NumOfRegisters = Len;
	MB->TX_MSG_LEN = 8;
	MB->RX_MSG_LEN = 5 + (2 * MB->NumOfRegisters);

	MB->TxBuffer[0] = SLA;
	MB->TxBuffer[1] = MB->Fcn;
	MB->TxBuffer[2] = (uint8_t)(MB->Reg_Address >> 8);
	MB->TxBuffer[3] = (uint8_t)(MB->Reg_Address);
	MB->TxBuffer[4] = (uint8_t)(MB->NumOfRegisters >> 8);
	MB->TxBuffer[5] = (uint8_t)(MB->NumOfRegisters);

	// assign the data ptr to the message
	MB->ptr = (void *)Data;
	MB_Master_Transmit(MB);
	return MB_STAT_OK;
}

MB_Status_t MB_Master_WriteHoldingReg(MB_Master_t *MB, uint8_t SLA, uint16_t RegAddress, uint16_t Data)
{
	// set the busy flag
	MB->Busy = 1;
	MB->Fcn = MB_PRST_SNG_REG;
	MB->SLA = SLA;
	MB->Reg_Address = RegAddress;
	MB->NumOfRegisters = 1;
	MB->TX_MSG_LEN = 8;
	MB->RX_MSG_LEN = 8;

	MB->TxBuffer[0] = SLA;
	MB->TxBuffer[1] = MB->Fcn;
	MB->TxBuffer[2] = (uint8_t)(MB->Reg_Address >> 8);
	MB->TxBuffer[3] = (uint8_t)(MB->Reg_Address);
	MB->TxBuffer[4] = (uint8_t)(Data >> 8);
	MB->TxBuffer[5] = (uint8_t)(Data);
	MB_Master_Transmit(MB);
	return MB_STAT_OK;
}

MB_Status_t MB_Master_WriteHoldingRegs(MB_Master_t *MB, uint8_t SLA, uint16_t RegAddress, uint16_t Len, uint16_t *Data)
{
	// set the busy flag
	MB->Busy = 1;
	MB->Fcn = MB_PRST_MUL_REGS;
	MB->SLA = SLA;
	MB->Reg_Address = RegAddress;
	MB->NumOfRegisters = Len;
	MB->TX_MSG_LEN = 9 + (2 * MB->NumOfRegisters);
	MB->RX_MSG_LEN = 8;

	MB->TxBuffer[0] = SLA;
	MB->TxBuffer[1] = MB->Fcn;
	MB->TxBuffer[2] = (uint8_t)(MB->Reg_Address >> 8);
	MB->TxBuffer[3] = (uint8_t)(MB->Reg_Address);
	MB->TxBuffer[4] = (uint8_t)(MB->NumOfRegisters >> 8);
	MB->TxBuffer[5] = (uint8_t)(MB->NumOfRegisters);
	MB->TxBuffer[6] = (uint8_t)(MB->NumOfRegisters * 2);

	for (uint16_t x = 0; x < MB->NumOfRegisters; x++)
	{
		MB->TxBuffer[7 + (2 * x)] = (uint8_t)(Data[x] >> 8);
		MB->TxBuffer[8 + (2 * x)] = (uint8_t)(Data[x]);
	}

	MB_Master_Transmit(MB);
	return MB_STAT_OK;
}

MB_Status_t MB_Master_ReadCoils(MB_Master_t *MB, uint8_t SLA, uint16_t RegAddress, uint16_t Len, uint8_t *Coils)
{
	// set the busy flag
	MB->Busy = 1;
	MB->Fcn = MB_READ_COIL;
	MB->SLA = SLA;
	MB->Reg_Address = RegAddress;
	MB->NumOfRegisters = Len;
	MB->TX_MSG_LEN = 8;
	MB->RX_MSG_LEN = 5 + (MB->NumOfRegisters / 8) + ((MB->NumOfRegisters % 8) ? 1 : 0);

	MB->TxBuffer[0] = SLA;
	MB->TxBuffer[1] = MB->Fcn;
	MB->TxBuffer[2] = (uint8_t)(MB->Reg_Address >> 8);
	MB->TxBuffer[3] = (uint8_t)(MB->Reg_Address);
	MB->TxBuffer[4] = (uint8_t)(MB->NumOfRegisters >> 8);
	MB->TxBuffer[5] = (uint8_t)(MB->NumOfRegisters);

	// assign the data ptr to the message
	MB->ptr = (void *)Coils;
	MB_Master_Transmit(MB);
	return MB_STAT_OK;
}

MB_Status_t MB_Master_ReadInputs(MB_Master_t *MB, uint8_t SLA, uint16_t RegAddress, uint16_t Len, uint8_t *Inputs)
{
	// set the busy flag
	MB->Busy = 1;
	MB->Fcn = MB_READ_INPUT_STAT;
	MB->SLA = SLA;
	MB->Reg_Address = RegAddress;
	MB->NumOfRegisters = Len;
	MB->TX_MSG_LEN = 8;
	MB->RX_MSG_LEN = 5 + (MB->NumOfRegisters / 8) + ((MB->NumOfRegisters % 8) ? 1 : 0);

	MB->TxBuffer[0] = SLA;
	MB->TxBuffer[1] = MB->Fcn;
	MB->TxBuffer[2] = (uint8_t)(MB->Reg_Address >> 8);
	MB->TxBuffer[3] = (uint8_t)(MB->Reg_Address);
	MB->TxBuffer[4] = (uint8_t)(MB->NumOfRegisters >> 8);
	MB->TxBuffer[5] = (uint8_t)(MB->NumOfRegisters);

	// assign the data ptr to the message
	MB->ptr = (void *)Inputs;
	MB_Master_Transmit(MB);
	return MB_STAT_OK;
}

MB_Status_t MB_Master_WriteCoil(MB_Master_t *MB, uint8_t SLA, uint16_t RegAddress, uint8_t Coil)
{
	// set the busy flag
	MB->Busy = 1;
	MB->Fcn = MB_FRC_SNG_COIL;
	MB->SLA = SLA;
	MB->Reg_Address = RegAddress;
	MB->NumOfRegisters = 1;
	MB->TX_MSG_LEN = 8;
	MB->RX_MSG_LEN = 8;

	MB->TxBuffer[0] = SLA;
	MB->TxBuffer[1] = MB->Fcn;
	MB->TxBuffer[2] = (uint8_t)(MB->Reg_Address >> 8);
	MB->TxBuffer[3] = (uint8_t)(MB->Reg_Address);
	MB->TxBuffer[4] = Coil ? 0xff : 0;
	MB->TxBuffer[5] = 0;
	MB_Master_Transmit(MB);
	return MB_STAT_OK;
}

MB_Status_t MB_Master_WriteCoils(MB_Master_t *MB, uint8_t SLA, uint16_t RegAddress, uint16_t Len, uint8_t *Coils)
{
	// set the busy flag
	MB->Busy = 1;
	MB->Fcn = MB_FRC_MUL_COILS;
	MB->SLA = SLA;
	MB->Reg_Address = RegAddress;
	MB->NumOfRegisters = Len;
	MB->TX_MSG_LEN = 9 + (MB->NumOfRegisters / 8) + ((MB->NumOfRegisters % 8) ? 1 : 0);
	MB->RX_MSG_LEN = 8;

	MB->TxBuffer[0] = SLA;
	MB->TxBuffer[1] = MB->Fcn;
	MB->TxBuffer[2] = (uint8_t)(MB->Reg_Address >> 8);
	MB->TxBuffer[3] = (uint8_t)(MB->Reg_Address);
	MB->TxBuffer[4] = (uint8_t)(MB->NumOfRegisters >> 8);
	MB->TxBuffer[5] = (uint8_t)(MB->NumOfRegisters);
	MB->TxBuffer[6] = (uint8_t)(MB->NumOfRegisters / 8) + ((MB->NumOfRegisters % 8) ? 1 : 0);

	for (uint16_t x = 0; x < MB->NumOfRegisters; x++)
	{
		MB_Encode_Coils(Coils, 0, MB->TxBuffer + 7, MB->NumOfRegisters);
	}

	MB_Master_Transmit(MB);
	return MB_STAT_OK;
}

MB_Status_t MB_Master_ReadWriteHoldingRegs(MB_Master_t *MB,
										   uint8_t SLA,
										   uint16_t ReadRegAddress,
										   uint16_t ReadLen,
										   uint16_t *ReadData,
										   uint16_t WriteRegAddress,
										   uint16_t WriteLen,
										   uint16_t *WriteData)
{
	// set the busy flag
	MB->Busy = 1;
	MB->Fcn = MB_PRST_READ_MUL_REGS;
	MB->SLA = SLA;
	MB->Reg_Address = ReadRegAddress;
	MB->Reg_Address_W = WriteRegAddress;
	MB->NumOfRegisters = ReadLen;
	MB->NumOfW_Regs = WriteLen;
	MB->TX_MSG_LEN = 13 + (2 * MB->NumOfW_Regs);
	MB->RX_MSG_LEN = 5 + (2 * MB->NumOfRegisters);

	MB->ptr = (void *)ReadData;

	MB->TxBuffer[0] = SLA;
	MB->TxBuffer[1] = MB->Fcn;
	MB->TxBuffer[2] = (uint8_t)(MB->Reg_Address >> 8);
	MB->TxBuffer[3] = (uint8_t)(MB->Reg_Address);
	MB->TxBuffer[4] = (uint8_t)(MB->NumOfRegisters >> 8);
	MB->TxBuffer[5] = (uint8_t)(MB->NumOfRegisters);

	MB->TxBuffer[6] = (uint8_t)(MB->Reg_Address >> 8);
	MB->TxBuffer[7] = (uint8_t)(MB->Reg_Address);
	MB->TxBuffer[8] = (uint8_t)(MB->NumOfRegisters >> 8);
	MB->TxBuffer[9] = (uint8_t)(MB->NumOfRegisters);

	MB->TxBuffer[10] = (uint8_t)(MB->NumOfW_Regs * 2);

	for (uint16_t x = 0; x < MB->NumOfW_Regs; x++)
	{
		MB->TxBuffer[11 + (2 * x)] = (uint8_t)(WriteData[x] >> 8);
		MB->TxBuffer[12 + (2 * x)] = (uint8_t)(WriteData[x]);
	}

	MB_Master_Transmit(MB);
	return MB_STAT_OK;
}


