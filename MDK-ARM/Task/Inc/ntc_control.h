/*
 * ntc_control.h
 *
 *  Created on: Sep 3, 2025
 *      Author: Tang
 */

#ifndef INC_NTC_CONTROL_H_
#define INC_NTC_CONTROL_H_

#include "temperature.h"
#include "pid.h"
#include "relay.h"
#include "stm32h7xx_hal.h"
#include <stdio.h>
#include <string.h>

void NTC_Control_Init(void);
void NTC_Control_Update(void);

#endif /* INC_NTC_CONTROL_H_ */
