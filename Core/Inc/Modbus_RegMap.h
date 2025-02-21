#ifndef Modbus_RegMap_h
#define Modbus_RegMap_h

#include "../../Drivers/CMSIS/ModBus_RTU/Modbus_Slave.h"
#include "NV_Params.h"

extern MB_Slave_t MB;

typedef struct HoldingRegs
{
    //holding registers will be divided into 2 parts

    //assume that the number of holding registers reserved for the 
    //system params is 128 words
    //part 1 ... is the system parameters
    // 0-127
    SystemParams_t sParams;
    uint16_t Dummy_1[128-(sizeof(SystemParams_t)/2)];


    //part 2 is the system states that need to be read/write
    uint16_t System_State; //128
    uint16_t FOC_State_Machine;
}HoldingRegs_t;
//////////////////////
typedef struct InputRegs
{
    //define all the system states (which are read-only)

    float Omege_E;
    float Theta_E;
    float Torque;

    float Ia;
    float Ib;
    float Ic;

    float Va;
    float Vb;
    float Vc;
//timing for each pusle FOC 
    float Ta;
    float Tb;
    float Tc;
//PWM
//PWM A
    float PWM_A_P;
    float PWM_A_N;
//PWM B
    float PWM_B_P;
    float PWM_B_N;
//PWM C
    float PWM_C_P;
    float PWM_C_N;

}InputRegs_t;
//////////////////
enum InputBits // 0 till 15
{
    //DRV8301 FAULTS 
    PWRGD,
    nOCTW,
    nFAULT
};
//////////////////
enum InputCoilBits // 0 till 15
{
    Enable_System,
    //DRV8301
    DRV_EN, //1
    //SPI SELECT SLAVE
    SPI_SS1,//2
    SPI_SS2,//2
    SPI_SS3//2

};
#define GetInputBit(bit) MB_Parse_Bit(MB.InputBits, bit)
#define SetInputBit(bit, State) MB_Encode_Bit(MB.InputBits, bit, State)

#define GetCoil(bit) MB_Parse_Bit(MB.CoilBits, bit)
#define SetCoil(bit, State) MB_Encode_Bit(MB.CoilBits, bit, State)

#define HoldingRegsSize (sizeof(HoldingRegs_t)/2)
#define InputRegsSize (sizeof(InputRegs_t)/2)

#endif
