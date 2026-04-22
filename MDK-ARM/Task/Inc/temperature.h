#ifndef __TEMPERATURE_H
#define __TEMPERATURE_H

#include "stm32h7xx_hal.h"
#include <math.h>
#include "adc.h"
#include <stdio.h>

void Temperature_Init(void);
float Read_Temperature(void);

#endif
