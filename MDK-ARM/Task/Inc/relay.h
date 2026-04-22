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

void Relay_Init();
void Relay_On();
void Relay_Off();
void Relay_Switch(uint8_t onoff);

#endif /* INC_RELAY_H_ */
