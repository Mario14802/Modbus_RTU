#ifndef _NV_Params_c
#define NV_Params_C

#include <stdint.h>

typedef struct
{
    //PID controller gains
    float Iq_Kp; //0,1
    float Iq_Ki; //2,3
    float Id_Kp; //4,5
    float Id_Ki; //6,7
    float Controller_Sampling_Time; //8,9

    //Current sensor filter constant
    float Idq_Filter_Const; //10,11

    //current sensor params for phase A and B
    float Ia_Offset_LSB; //12,13
    float Ia_Gain_LSB_A; //14,15
    float Ib_Offset_LSB; //16,17
    float Ib_Gain_LSB_A; //18,19

    //rotor position sensor
    float angle_offset; //20,21
    float peak_sine;//22,23
    float peak_cosine;//24,25
   
    //rotor position sensor params
    float RotorPosOffset_Deg; //26,27

    float I_Kt; //28,29 limiter  
    float I_sat;// 30,31 current saturation level + and - 
    
    //filter constants
    float LPF_Idq_Fc;//32,33
    float LPF_Iq_Fc;//34,35
    float LPF_Id_Fc;//36,37

    uint16_t Checksum; //38,39

}SystemParams_t;

#define SystemParams_Size_Words (sizeof(SystemParams_t)/2)





#endif
