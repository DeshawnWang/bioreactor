/*
 * stir.h
 *
 *  Created on: Aug 27, 2025
 *      Author: Tang
 */

#ifndef INC_STIR_H_
#define INC_STIR_H_

#include "stm32h7xx_hal.h"
#include <stdint.h>
#include "usart.h"



void Get_Sign();
void Start_Stir();
void Stop_Stir();
void SpeedMode();

#endif /* INC_STIR_H_ */
