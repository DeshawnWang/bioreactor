/*
 * stir.h
 *
 *  Created on: Aug 27, 2025
 *      Author: Tang
 */

#ifndef INC_STIR_H_
#define INC_STIR_H_

#include <stdint.h>
#include "usart.h"     // 用于 huart1 外部声明
#include "stdio.h"
#include "crc.h"        // 用于 CRC16 校验


void Process_Stir_Command(uint8_t *buffer, uint16_t size);
void Set_Stir_Speed(int16_t speed);
void Get_Sign(void);
void SpeedMode(void);
int32_t Read_Stir_Speed(void);

#endif /* INC_STIR_H_ */
