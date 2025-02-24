from pymodbus.client import ModbusSerialClient
import time

# Modbus coil and discrete input addresses
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

    # Step 1: Write registers (for testing, writing 8 values starting at register 0)
    response = client.write_registers(address=0, values=[1, 2, 3, 4, 5, 6, 7, 8], slave=1)
    if response.isError():
        print("Error writing registers:", response)
    else:
        print("Registers written successfully.")

    # Step 2: Trigger the NV save operation by setting the corresponding coil
    response = client.write_coil(address=MB_COIL_SAVE_NV, value=True, slave=1)
    if response.isError():
        print("Error triggering NV save operation:", response)
    else:
        print("NV save command sent.")

    # Allow time for the MCU to process the save command and clear the coil
    time.sleep(1)

    # Step 3: Read back the NV save coil to verify that the NV save process has completed
    response = client.read_coils(address=MB_COIL_SAVE_NV, count=1, slave=1)
    if response.isError():
        print("Error reading NV save coil:", response)
    else:
        print("NV save coil state:", response.bits[0])

    # If the NV save coil is cleared, then the EEPROM write has been performed.
    if response.bits[0] is False:
        # --- Read checksum after writing/saving ---
        response_checksum = client.read_holding_registers(address=38, count=2, slave=1)
        if response_checksum.isError():
            print("Error reading checksum registers after NV save:", response_checksum)
        else:
            reg38 = response_checksum.registers[0]
            reg39 = response_checksum.registers[1]
            combined_checksum = (reg39 << 16) | reg38
            print("Checksum after NV save:")
            print(" Combined: {} (size: 4 bytes)".format(reg38, reg39, combined_checksum))

        # --- Reset and load NV data ---
        # Trigger chip reset
        response = client.write_coil(address=MB_COIL_RESET, value=True, slave=1, no_response_expected=True)
        if response.isError():
            print("Error triggering chip reset:", response)
        else:
            print("Chip reset command sent.")
        time.sleep(1)

        # Trigger the NV load operation
        response = client.write_coil(address=MB_COIL_LOAD_NV, value=True, slave=1)
        if response.isError():
            print("Error triggering NV load operation:", response)
        else:
            print("NV load command sent.")
        time.sleep(0.5)

        # --- Read checksum after loading NV data ---
        response_checksum = client.read_holding_registers(address=38, count=2, slave=1)
        if response_checksum.isError():
            print("Error reading checksum registers after NV load:", response_checksum)
        else:
            reg38 = response_checksum.registers[0]
            reg39 = response_checksum.registers[1]
            combined_checksum = (reg39 << 16) | reg38
            print("Checksum after NV save:")
            print(" Combined: {} (size: 4 bytes)".format(reg38, reg39, combined_checksum))

        # Read the discrete input to check the checksum error flag
        response = client.read_discrete_inputs(address=MB_INPUT_ERROR_CHECKSUM, count=1, slave=1)
        if response.isError():
            print("Error reading checksum error flag:", response)
        else:
            if response.bits[0]:
                print("Checksum error: the computed checksum is not equal!")
            else:
                print("Checksum OK: the computed checksum matches!")
    else:
        print("NV save operation did not complete (save coil still active).")

    # Close the connection
    client.close()
else:
    print("Failed to connect to Modbus Slave!")
