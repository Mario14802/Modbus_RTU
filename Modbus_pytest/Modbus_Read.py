from pymodbus.client import ModbusSerialClient
import time

# Modbus addresses definitions
MB_REGISTERS_START    = 0  # Starting address for registers
MB_REGISTERS_COUNT    = 8  # Number of registers to read

MB_COIL_SAVE_NV = 5         # Trigger non-volatile save
MB_COIL_LOAD_NV = 6         # Trigger non-volatile load
MB_COIL_RESET = 7           # Trigger chip reset
MB_INPUT_ERROR_CHECKSUM = 3 # Discrete input for checksum error flag

# Configure the Modbus RTU client
client = ModbusSerialClient(
    port="COM7",       # Change to your port (e.g., "COM3" on Windows)
    baudrate=115200,
    timeout=1
)

if client.connect():
    print("Connected to Modbus Slave!")
    
    # Step 0: Trigger the NV load operation
    response = client.write_coil(address=MB_COIL_LOAD_NV, value=True, slave=1)
    if response.isError():
        print("Error triggering NV load operation:", response)
    else:
        print("NV load command sent.")
    time.sleep(0.5)  # Allow time for NV load to complete

    # Step 1: Read holding registers (for example, 8 registers starting at address 0)
    response = client.read_holding_registers(address=MB_REGISTERS_START, count=MB_REGISTERS_COUNT, slave=1)
    response = client.read_holding_registers(address=34, count=5, slave=1)
    if response.isError():
        print("Error reading registers:", response)
    else:
        print("Read registers:", response.registers)
    
    # Step 2: Read a single coil's status
    response = client.read_coils(address=MB_COIL_LOAD_NV, count=1, slave=1)
    if response.isError():
        print("Error reading coil status:", response)
    else:
        print("Coil status:", response.bits[0])
    
    # Step 3: Read a discrete input status
    response = client.read_discrete_inputs(address=MB_INPUT_ERROR_CHECKSUM, count=1, slave=1)
    if response.isError():
        print("Error reading discrete input:", response)
    else:
        print("CheckSum input status if false working then the checksum matches:", response.bits[0])
    
    # Close the connection after reading
    client.close()
else:
    print("Failed to connect to Modbus Slave!")
