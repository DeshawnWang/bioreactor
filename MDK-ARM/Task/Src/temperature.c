#include "temperature.h"

// NTC 阻值计算参数（10kΩ@25℃，B=3950，下拉电阻假设为10kΩ）
#define R_PULL 10000.0f    // 下拉电阻阻值（Ω）
#define R0     10000.0f    // NTC 标称阻值（Ω），25℃时 10kΩ
#define T0     298.15f     // 25℃ = 298.15K
#define BETA   3950.0f     // NTC B 值
#define VREF   3.3f        // ADC 参考电压



// 初始化 ADC 通道
void TempSensor_Init(void) {
    // 这里假设在 CubeMX 中已配置 ADC1 及对应通道，本函数可留空
}

// 读取并计算温度（℃），同时对 ADC 原始值进行简单低通滤波
float Read_Temperature(void) {
    // 启动ADC转换
    HAL_ADC_Start(&hadc2);
    // 关键修改：将HAL_MAX_DELAY改为100ms超时，避免永久阻塞
    if (HAL_ADC_PollForConversion(&hadc2, 100) != HAL_OK) {
        HAL_ADC_Stop(&hadc2);
        return -100.0f;  // 超时返回错误温度值，避免循环卡死
    }
    uint32_t raw = HAL_ADC_GetValue(&hadc2);  // 16位ADC原始值（0..65535）

    // 简单滤波：IIR低通滤波器
    static float filt = 0;
    filt += ((float)raw - filt) * 0.0625f;   // α = 1/16
    float adc_value = filt;

    // 将ADC值换算为电压
    float Vadc = (adc_value / 65535.0f) * VREF;
    if (Vadc <= 0.0f || Vadc >= VREF) {  // 增加电压范围判断，避免异常值
        HAL_ADC_Stop(&hadc2);
        return -100.0f;
    }

    // 关键修改：修正NTC阻值计算公式（匹配“VREF→下拉电阻→NTC→GND”接线）
    float Rntc = R_PULL * (Vadc / (VREF - Vadc));

    // 使用B值公式计算温度
    float invT = (1.0f / T0) + (1.0f / BETA) * logf(Rntc / R0);
    float Tkelvin = 1.0f / invT;
    float Tc = Tkelvin - 273.15f;  // 转换为摄氏度

    // 限定温度范围（0–150℃）
    if (Tc < 0.0f) Tc = 0.0f;
    if (Tc > 150.0f) Tc = 150.0f;

    HAL_ADC_Stop(&hadc2);  // 补充ADC停止，避免资源占用
    return Tc;
}
