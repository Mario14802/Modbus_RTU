#ifndef MODBUS_CRC_H
#define MODBUS_CRC_H

#ifdef __cplusplus
 extern "C" {
#endif

#include <stdint.h>
#include <string.h>
#include <stdbool.h>

typedef enum {
	MB_STAT_OK,
	MB_INIT_OK,
	MB_NOINIT,
	MB_INIT_ERR_INVALID_SLA,
	MB_INIT_ERR_MEM_ALLOC,
	MB_INIT_ERR_FN_PTR,

	MB_TX_ERR_Invalid_PTR,
	MB_TX_ERR_HW,
	MB_TX_ERR_INVALID_FCN,

	MB_RX_ERR_NoData,
	MB_RX_ERR_FUN,
	MB_RX_ERR_ADD,
	MB_RX_ERR_VAL,
	MB_RX_ERR_CRC,
	MB_RX_OVF,
	MB_RX_TIMEOUT,
	MB_RX_WAITING_MSG,

	MB_EXC_RESPONSE,
} MB_Status_t;

typedef enum {
	MB_READ_COIL = 0x1,
	MB_READ_INPUT_STAT = 0x2,
	MB_READ_MUL_HLD_REG = 0x3,
	MB_READ_INPUT_REG = 0x4,
	MB_FRC_SNG_COIL = 0x5,
	MB_PRST_SNG_REG = 0x6,
	//MB_READ_EXCP_STAT = 07,
	MB_FRC_MUL_COILS = 0x0f,
	MB_PRST_MUL_REGS = 0x10,
	//the non standard read while write command
	MB_PRST_READ_MUL_REGS = 0x17
} MB_FC_t;

#define MB_EXC_LEN 5

typedef enum {
	MB_NO_EXEPTION,
	MB_ILLEGAL_FUNC = 1,
	MB_ILLEGAL_DATA_ADDRESS = 2,
	MB_ILLEGAL_DATA_VALUE = 3,
	MB_SALVE_DEVICE_FAILURE = 4,
	MB_ACK = 5,
	MB_SLAVE_BUSY = 6
} MB_EXC_CODE;


uint16_t crc16(uint8_t *buffer, uint16_t buffer_length);

/*
 *
 * Helper Function Section
 * Used to encode and decode data from and to the bus
 *
 */

//decoding functions
bool MB_Parse_Bit(uint8_t *data, uint16_t Address);
void MB_Parse_Coils(uint8_t *data, uint16_t Offset, uint8_t *Output,
		uint16_t Length);
void MB_Parse_UInt8(uint8_t *data, uint16_t *offset, uint8_t *Output);
void MB_Parse_Int8(uint8_t *data, uint16_t *offset, int8_t *Output);
void MB_Parse_UInt16(uint8_t *data, uint16_t *offset, uint16_t *Output);
void MB_Parse_Int16(uint8_t *data, uint16_t *offset, int16_t *Output);
void MB_Parse_UInt32(uint8_t *data, uint16_t *offset, uint32_t *Output);
void MB_Parse_Int32(uint8_t *data, uint16_t *offset, int32_t *Output);
void MB_Parse_Float(uint8_t *data, uint16_t *offset, float *Output);

//encoding functions
void MB_Encode_Bit(uint8_t *data, uint16_t Address, bool bit);
void MB_Encode_Coils(uint8_t *data, uint16_t Offset, uint8_t *Output,
		uint16_t Length);
void MB_Encode_UInt8(uint8_t *data, uint8_t Input, uint16_t *Offset);
void MB_Encode_Int8(uint8_t *data, int8_t Input, uint16_t *Offset);
void MB_Encode_UInt16(uint8_t *data, uint16_t Input, uint16_t *Offset);
void MB_Encode_Int16(uint8_t *data, int16_t Input, uint16_t *Offset);
void MB_Encode_Float(uint8_t *data, float Input, uint16_t *Offset);

void MB_Encode_Float_Reg(uint16_t *reg, uint16_t *Offset, float Value);

void MB_Encode_Int32_Reg(uint16_t *reg, uint16_t *Offset, int32_t Value);

void MB_Encode_Uint32_Reg(uint16_t *reg, uint16_t *Offset, uint32_t Value);

void MB_Decode_Float_Reg(uint16_t *reg, uint16_t *Offset, float *Value);
void MB_Decode_Int32_Reg(uint16_t *reg, uint16_t *Offset, int32_t *Value);
void MB_Decode_Uint32_Reg(uint16_t *reg, uint16_t *Offset, uint32_t *Value);

#ifdef __cplusplus
 }
#endif

#endif
