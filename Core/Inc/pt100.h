/*
 * pt100.h
 *
 *  Created on: Nov 5, 2025
 *      Author: Tang
 */

#ifndef INC_PT100_H_
#define INC_PT100_H_

#include "usart.h"

// 添加温度滤波定义
#define TEMP_FILTER_SAMPLES 5  // 滤波窗口大小

typedef struct {
    float temperature;
    float history[TEMP_FILTER_SAMPLES];
    uint8_t index;
    uint8_t count;
} TempFilter_t;

extern uint8_t rx_data8[RX_BUFFER_SIZE];    // uart7接收缓存
extern uint8_t rx_pt100_flag;
extern uint8_t tx_pt100_flag;
extern float pt100_temp;
extern TempFilter_t temp_filter;

// 函数声明
void PT100_Init(void);
void PT100_GetTemperature(void);
void PT100_ReadTemperature(void);
float PT100_Task(void);
uint16_t PT100_CRC16(uint8_t *data, uint16_t len);
void TempFilter_Init(void);
float TempFilter_Update(float new_temp);

#endif /* INC_PT100_H_ */
