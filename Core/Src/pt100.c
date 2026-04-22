#include "pt100.h"
#include <string.h>
#include <stdio.h>

// 缓冲区定义
uint8_t rx_data8[RX_BUFFER_SIZE];    // uart8接收缓存
uint8_t rx_pt100_flag = 0;
uint8_t tx_pt100_flag = 0;
float pt100_temp = 0.0f;
TempFilter_t temp_filter;

// 温度滤波初始化
void TempFilter_Init(void) {
    memset(&temp_filter, 0, sizeof(temp_filter));
    for(int i = 0; i < TEMP_FILTER_SAMPLES; i++) {
        temp_filter.history[i] = 0.0f;
    }
}

// 温度滤波更新
float TempFilter_Update(float new_temp) {
    // 更新历史数据
    temp_filter.history[temp_filter.index] = new_temp;
    temp_filter.index = (temp_filter.index + 1) % TEMP_FILTER_SAMPLES;

    if(temp_filter.count < TEMP_FILTER_SAMPLES) {
        temp_filter.count++;
    }

    // 计算移动平均
    float sum = 0.0f;
    for(int i = 0; i < temp_filter.count; i++) {
        sum += temp_filter.history[i];
    }

    temp_filter.temperature = sum / temp_filter.count;
    printf("Temp=%0.3f",temp_filter.temperature);
    return temp_filter.temperature;
}

// PT100初始化
void PT100_Init(void)
{
    // 初始化温度滤波器
    TempFilter_Init();

    // 初始化UART8接收
    HAL_UARTEx_ReceiveToIdle_DMA(&huart8, rx_data8, sizeof(rx_data8));
    __HAL_DMA_DISABLE_IT(huart8.hdmarx, DMA_IT_HT);

    printf("PT100 Temperature Module Initialized\r\n");
}

// 获取温度数据
void PT100_GetTemperature(void)
{
	// 清空接收缓存
	memset(rx_data8, 0, sizeof(rx_data8));
	uint8_t temp_cmd[] = {0x01,       // 设备地址
		0x03,       // 功能码: 读取保持寄存器
		0x00, 0x00, // 起始地址: 0x0000
		0x00, 0x02, // 寄存器数量: 2个 (32位)
		0xC4, 0x0B  // CRC校验
	};
	tx_pt100_flag = 0;

	// 发送读取命令
	HAL_UART_Transmit_DMA(&huart8, temp_cmd, sizeof(temp_cmd));
	HAL_UARTEx_ReceiveToIdle_DMA(&huart8, rx_data8, sizeof(rx_data8));
}

// 读取并解析温度数据
void PT100_ReadTemperature(void)
{
    uint16_t len = sizeof(rx_data8);
    uint8_t found = 0;
//
//    // 调试打印：查看接收缓冲区
//    printf("PT100 RX dump: ");
//    for (uint16_t i = 0; i < 16 && i < len; i++) {
//        printf("%02X ", rx_data8[i]);
//    }
//    printf("\r\n");

    // 查找有效的Modbus响应帧 (设备地址0x01, 功能码0x03)
    for (uint16_t i = 0; i < len - 6; i++) {
        if (rx_data8[i] == 0x01 &&
        		rx_data8[i+1] == 0x03 &&
				rx_data8[i+2] == 0x04) // 数据长度4字节
        {
            found = 1;
            printf("找到PT100温度数据帧，位置: %d\r\n", i);
//
//            // 打印完整响应帧
//            printf("PT100原始报文: ");
//            for (uint16_t j = i; j < i + 9 && j < len; j++) {
//                printf("%02X ", rx_data8[j]);
//            }
//            printf("\r\n");

            // 解析32位温度数据
            int32_t temp_raw = ((int32_t)rx_data8[i+3] << 24) |
                              ((int32_t)rx_data8[i+4] << 16) |
                              ((int32_t)rx_data8[i+5] << 8) |
                              (int32_t)rx_data8[i+6];

            // 转换为浮点温度值 (单位: 0.0001℃)
            pt100_temp = (float)temp_raw * 0.0001f;

            printf("PT100_temp: %.4f ℃\r\n", pt100_temp);
            break;
        }
    }

    if (!found) {
        printf("未找到有效的PT100温度数据\r\n");
    }
}

// PT100任务函数
float PT100_Task(void)
{
    PT100_GetTemperature();

    // 等待数据到达 (最多200ms)
    uint32_t start = HAL_GetTick();
    while (!rx_pt100_flag && (HAL_GetTick() - start < 200)) {}

    PT100_ReadTemperature();
    rx_pt100_flag = 0;

    // 重新启动DMA接收
    HAL_UARTEx_ReceiveToIdle_DMA(&huart8, rx_data8, sizeof(rx_data8));

    // 应用滤波并返回滤波后的温度
    return TempFilter_Update(pt100_temp);
}

// Modbus CRC16校验计算
uint16_t PT100_CRC16(uint8_t *data, uint16_t len)
{
    uint16_t crc = 0xFFFF;
    while (len--) {
        crc ^= *data++;
        for (uint8_t i = 0; i < 8; i++) {
            if (crc & 0x0001) {
                crc = (crc >> 1) ^ 0xA001;
            } else {
                crc >>= 1;
            }
        }
    }
    return crc;
}
