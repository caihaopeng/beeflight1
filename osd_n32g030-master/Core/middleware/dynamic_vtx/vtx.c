/**
 * @file vtx.c
 * @author Gordon Mak (mgd@sequoiatech.co)
 * @brief
 * @version 0.1
 * @date 2021-08-12
 *
 * @copyright Copyright (c) 2021
 *
 */
#include <stdbool.h>
#include <stdint.h>

#include "PID.h"
#include "vtx.h"

static float __scaleRangef(float x, float srcMin, float srcMax, float destMin, float destMax);

static float temperature;

static PIDController pid = {PID_KP, PID_KI, PID_KD,
                            PID_TAU,
                            PID_LIM_MIN, PID_LIM_MAX,
                            PID_LIM_MIN_INT, PID_LIM_MAX_INT,
                            (float)VTX_TASK_PERIOD / 1000};

uint16_t VTx_PID_Converter(vtxPower_e power)
{
    float temp = 0;

    switch (power) {
        case VTX_POWER_5MW:
            temp = POWER_5MW_VOLTAGE_VAL;
            break;

        case VTX_POWER_25MW:
            temp = POWER_25MW_VOLTAGE_VAL;
            break;

        case VTX_POWER_MAX:
            PIDController_Update(&pid, PID_SETPOINT, temperature);
            temp = pid.out;
            break;

        default:
            break;
    }

#if defined(_VTX_TRIODE_TYPE_NPN)
    return (uint16_t)__scaleRangef(temp, 0, POWER_MAX_VOLTAGE_VAL, 0, VTX_RESOULTION_VAL);
#elif defined(_VTX_TRIODE_TYPE_PNP)
    return (uint16_t)__scaleRangef(temp, 0, POWER_MAX_VOLTAGE_VAL, VTX_RESOULTION_VAL, 0);
#endif
}

void VTx_PID_SetTemperature(float temp)
{
    temperature = temp;
}

static float __scaleRangef(float x, float srcMin, float srcMax, float destMin, float destMax)
{
    float a = (destMax - destMin) * (x - srcMin);
    float b = srcMax - srcMin;
    return (a / b) + destMin;
}
