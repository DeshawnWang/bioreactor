/*
 * pump.h
 *
 *  Created on: Aug 21, 2025
 *      Author: Tang
 */

#ifndef INC_PUMP_H_
#define INC_PUMP_H_

#include <stdint.h>
#include "usart.h"
#include <stdio.h>
#include <string.h>


void Start_AcidPump(void);
void Start_AlkaliPump(void);
void Stop_AcidPump(void);
void Stop_AlkaliPump(void);

#endif /* INC_PUMP_H_ */
