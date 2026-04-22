/*
 * ph.h
 *
 *  Created on: Aug 27, 2025
 *      Author: Tang
 */

#ifndef INC_PH_H_
#define INC_PH_H_

#include <stdint.h>
#include "usart.h"
#include <string.h>
#include <stdio.h>
#include "pump.h"

void requestPH(void);
void readPH(void);
void adjustPH(void);
void Task_PH(void);

#endif /* INC_PH_H_ */
