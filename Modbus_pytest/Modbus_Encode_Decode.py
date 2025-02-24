import struct
import numpy as np
from pymodbus.client import ModbusSerialClient
 
port = 'COM9'
baud = 115200
SLA = 1
 
def main():
   
    client = ModbusSerialClient(port=port,baudrate=baud)
    Result = client.read_holding_registers(0,2,SLA)
   
   
    Res = Get_Float(Result.registers,0)
   
    Data = 10.5
   
    HoldingRegisters = Encode_Float(Data)
   
    #set the Iq_Kp
    Iq_Kp = 20.5
    HoldingRegisters = Encode_Float(Iq_Kp)
    Result = client.write_registers(0,HoldingRegisters,SLA)
   
    Result = client.read_holding_registers(0,2,SLA)
    F = Get_Float(Result.registers,0)
    pass
 
def Get_Float(Registers:list[int],offset:int)->float:
    Low = Registers[offset]
    High = Registers[offset+1]
    Equivalent_Value = int(Low) | (int(High)<<16)
    return struct.unpack('f',struct.pack('I',Equivalent_Value))[0]
 
def Encode_Float(Input:float)->list[np.uint16]:
    #convert float into 4-bytes equivalent value
    # ____Float____  --> __ __ __ __ bytes
    floatBytes = struct.pack('f',Input)
    #pack those 4 bytes into a 32-bit integer
    # __ __ __ __ bytes  --> ____int 32____
    Equivalnt_Integer = struct.unpack('I',floatBytes)[0]
    #split the interger into 2 uint16 registers
    #____int32____   --> ____ ____ uint16
    Low = np.uint16(Equivalnt_Integer&0xffff)
    High = np.uint16((Equivalnt_Integer>>16)&0xffff)
    return [Low,High]
 
if __name__ == '__main__':
    main()