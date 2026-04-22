/*
 * pwm2.h
 *
 *  Created on: Nov 7, 2025
 *      Author: Tang
 */

#ifndef INC_PWM2_H_
#define INC_PWM2_H_

#include "tim.h"
#include "ads8688.h"
#include <stdio.h>

void Set_PWM_Duty(float duty_percent);
void Pump_Control_Update(void);
void PWM2_Init(void);

#endif /* INC_PWM2_H_ */
