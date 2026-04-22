#include "temperature.h"
#include <math.h>
#include <stdio.h>

#define R_PULL 10000.0f
#define R0     10000.0f
#define T0     298.15f
#define BETA   3950.0f
#define VREF   3.3f

//// 简化滤波：只使用一阶低通滤波
//void TempSensor_Init(void) {
//    HAL_ADC_Start(&hadc2);
//}
//
//// 改进的温度读取函数
//float Read_Temperature(void) {
//    HAL_ADC_Start(&hadc2);
//    if (HAL_ADC_PollForConversion(&hadc2, 100) != HAL_OK) {
//        HAL_ADC_Stop(&hadc2);
//        return -100.0f;
//    }
//
//    uint32_t raw = HAL_ADC_GetValue(&hadc2);
//
//    // 简化滤波：只使用一阶低通滤波，减少滞后
//    static float temp_filt = 25.0f;
//    float Vadc = ((float)raw / 65535.0f) * VREF;
//
//    if (Vadc <= 0.001f || Vadc >= VREF - 0.001f) {
//        HAL_ADC_Stop(&hadc2);
//        return -100.0f;
//    }
//
//    // NTC计算
//    float Rntc = R_PULL * (Vadc / (VREF - Vadc));
//    float invT = (1.0f / T0) + (1.0f / BETA) * logf(Rntc / R0);
//    float Tc = (1.0f / invT) - 273.15f;
//
//    // 温度范围限制
//    if (Tc < -20.0f) Tc = -20.0f;
//    if (Tc > 150.0f) Tc = 150.0f;
//
//    // 温和的一阶低通滤波（系数0.2，响应更快）
//    temp_filt += (Tc - temp_filt) * 0.2f;
//
//    HAL_ADC_Stop(&hadc2);
//    return temp_filt;
//}
