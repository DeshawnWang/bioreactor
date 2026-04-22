/*
 * relay.h
 *
 *  Created on: Sep 2, 2025
 *      Author: Tang
 */

#ifndef INC_RELAY_H_
#define INC_RELAY_H_

#include "usart.h"
#include "gpio.h"
#include "main.h"
#include <stdio.h>

void Relay_Init(void);
void Relay_On(void);
void Relay_Off(void);
void Relay_Switch(uint8_t onoff);
uint8_t Get_Relay_Status(void);  // 新增状态获取函数

#endif /* INC_RELAY_H_ */
