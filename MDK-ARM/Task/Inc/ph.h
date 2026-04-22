/*
 * ph.h
 *
 *  Created on: Aug 27, 2025
 *      Author: Tang
 */

#ifndef INC_PH_H_
#define INC_PH_H_

#include "stm32h7xx_hal.h"
#include <stdint.h>
#include "usart.h"
#include <string.h>
#include <stdio.h>
#include "pump.h"

void requestPH();
void readPH();
void adjustPH();
void Task_PH();

#endif /* INC_PH_H_ */
