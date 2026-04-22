/*
 * flower.h
 *
 *  Created on: Mar 13, 2026
 *      Author: PIGflyme
 */

#ifndef INC_FLOWER_H_
#define INC_FLOWER_H_

#include "usart.h"
#include "crc.h"
#include <stdio.h>
#include <string.h>
void Flower_SetTarget(float target_lpm);
void Flower_ReadFlow(void);
void Flower_Parse(uint8_t *buf);
void Task_Flow(void);

#endif /* INC_FLOWER_H_ */
