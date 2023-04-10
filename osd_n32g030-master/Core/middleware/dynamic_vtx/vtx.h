/**
 * @file vtx.h
 * @author Gordon Mak (mgd@sequoiatech.co)
 * @brief 
 * @version 0.1
 * @date 2022-03-01
 * 
 * @copyright Copyright (c) 2022
 * 
 */
#ifndef __VTX_H
#define __VTX_H

/**
 * @brief VTx Output Triode Type (Select one of them)
 */
#define _VTX_TRIODE_TYPE_PNP
// #define _VTX_TRIODE_TYPE_NPN

typedef enum {
    VTX_POWER_5MW   = 0,
    VTX_POWER_25MW  = 1,
    VTX_POWER_MAX   = 2,
} vtxPower_e;

/**
 * @brief Task period time
 */
#define VTX_TASK_PERIOD             500  // ms, match with SAMPLE_TIME_S
#define VTX_RESOULTION_VAL          720  // 3.6V * (100 * 2), 1 step = 0.05V
#define VTX_PWM_FREQ                8000 // 8 KHz

/**
 * @brief VTx Power to voltage table
 */
#define POWER_5MW_VOLTAGE_VAL       (0.0f)
#define POWER_25MW_VOLTAGE_VAL      (0.22f)
#define POWER_MAX_VOLTAGE_VAL       (1.2f)

/**
 * @brief PID parameter
 */
#define PID_SETPOINT                (75.0f) // temperature setpoint

#define PID_KP                      (0.25f)
#define PID_KI                      (0.04f)
#define PID_KD                      (0.005f)

#define PID_TAU                     (0.01f)

#define PID_LIM_MIN                 POWER_25MW_VOLTAGE_VAL // PID min value
#define PID_LIM_MAX                 (1.19f)                // PID max value

#define PID_LIM_MIN_INT             (0.0f)
#define PID_LIM_MAX_INT             (2.0f)

uint16_t VTx_PID_Converter(vtxPower_e power);
void VTx_PID_SetTemperature(float temp);

#endif /* __VTX_H */
