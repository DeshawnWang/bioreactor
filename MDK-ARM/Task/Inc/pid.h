/*
 * pid.h
 *
 *  Created on: Sep 3, 2025
 *      Author: Tang
 */


#ifndef INC_PID_H_
#define INC_PID_H_

#include <math.h>
#include <stdio.h>
#include "stm32h7xx_hal.h"

// ---------------- PID 结构体 ----------------
typedef struct {
    float Kp, Ki, Kd;
    float prev_error;
    float integral;
    float out_min, out_max;

    // 调试相关
    float derivative;
    float last_output;
} PID_t;

typedef struct {
    float Kp, Ki, Kd;
} PID_Params_t;

// ---------------- API ----------------
void PID_Init(PID_t *pid, float Kp, float Ki, float Kd, float out_min, float out_max);
float PID_Task(PID_t *pid, float setpoint, float measurement, float dt);
void PID_AutoTune(PID_Params_t *params,
                  float (*readTemp)(void),
                  void (*relayCtrl)(uint8_t),
                  float target_temp,
                  uint32_t duration_ms);




#endif /* INC_PID_H_ */
