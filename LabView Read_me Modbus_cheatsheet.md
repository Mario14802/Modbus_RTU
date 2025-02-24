#Modbus
##Simple cheatsheet for using Modbus

**NOTE: All iterations start from zero**

- MB_Input_Error_Checksum =         3 ------>             to check for errors
- MB_COIL_SAVE_NV =                 5 ------>             Trigger non-volatile save (Send 1 on address 5 to Save data on flash memory)
- MB_COIL_LOAD_NV =                 6 ------>             Trigger non-volatile load (Send 1 on address 6 to Load data)
- Led_Blink =                       8 ------>             To test coil by using LED send request on address 8

#Steps on how to (save and load) data From MCU using Modbus used only when you want to save data on MCU :
1. Write the data to the holding register parameters.  
2. Send 1 to address 5 to save the data.  
3. To load the data, send 1 to address 6.  
4. To check for errors and verify if the data was loaded correctly, read address 3:  
   - If the value is 0, the data is correct.  
   - If the value is 1, the data is incorrect.