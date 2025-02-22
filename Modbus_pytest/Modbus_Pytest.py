from pymodbus.client import ModbusSerialClient
import time

MB_Coil_Save_NV_Variables=5
MB_Coil_Reset_Chip=7
MB_Coil_Load_NV_Variables=6
MB_Input_Error_Checksum=3
# Configure the Modbus RTU client
client = ModbusSerialClient(
    port="COM7",  # Change to COMx for Windows (e.g., "COM3")
    baudrate=115200,
    timeout=1
)

# Connect to the slave device
if client.connect():
    print("Connected to Modbus Slave!")

    # Read holding registers (Function Code 0x03)
    # response = client.read_holding_registers(address=0, count=5,slave=1)  # Change address/unit if needed
    response = client.write_registers(address=0,values=[1,2,3,4,5,6,7,8],slave=1)
    response=client.write_coil(address=MB_Coil_Save_NV_Variables,value=False,slave=1)

    if not response.isError():
        print("Register Values:", response.registers)
    else:
        print("Error reading registers:", response)
    time.sleep(1)
    response=client.read_coils(address=MB_Coil_Save_NV_Variables,count=1,slave=1)

    if response.bits[0]==False:
        response=client.write_coil(address=MB_Coil_Reset_Chip,value=True,slave=1,no_response_expected=True)
        time.sleep(1)
        response=client.write_coil(address=MB_Coil_Load_NV_Variables,value=True,slave=1)
        time.sleep(0.5)
        response=client.read_coils(address=MB_Coil_Load_NV_Variables,count=1,slave=1)
        response=client.read_holding_registers(address=0,count=8,slave=1)
        response=client.read_discrete_inputs(address=MB_Input_Error_Checksum,count=1,slave=1)
    else:
        pass

    # Close connection
    client.close()
else:
    print("Failed to connect to Modbus Slave!")
