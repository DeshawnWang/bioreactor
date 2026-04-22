#include "ph.h"
#include "pump.h"
#include "main.h"
#include <stdio.h>

#define PUMP_RUNTIME      5000   // 泵运行时间 5 秒
#define PUMP_COOLDOWN     30000   // 泵冷却时间 30 秒

float ph_set = 7.0f;        // 默认目标 pH
float ph_current = 0.0f;    // 实时 pH
float ph_read = 7.0f;       // 从报文解析的 pH
float deadband = 0.5f;      // pH 调节死区

uint8_t pump_running = 0;
uint32_t pump_start_time = 0;
uint32_t pump_stop_time = 0;

uint8_t rx_data3[RX_BUFFER_SIZE];   // USART3 接收缓存（确保 RX_BUFFER_SIZE ≥ 9）
uint8_t rx_ph_flag = 0;            // USART3 接收完成标志（DMA 完成后置1）
uint8_t tx_ph_flag = 0;            // USART3 发送完成标志（DMA 完成后置1）


// ========== 发送 pH 请求 ==========
void requestPH(void) {
    uint8_t requestPH[] = {0x01,0x03,0x00,0x64,0x00,0x02,0x85,0xD4};
    tx_ph_flag = 0;
    HAL_UART_Transmit_DMA(&huart3, requestPH, sizeof(requestPH));

    uint32_t timeout = HAL_GetTick();
    while(tx_ph_flag == 0 && (HAL_GetTick() - timeout) < 1000) {}
    if(tx_ph_flag == 0) printf("pH send timeout\r\n");
}

// ========== 解析 pH 报文 ==========
void readPH(void) {
    if(rx_data3[0] == 0x01 && rx_data3[1] == 0x03 && rx_data3[2] == 0x04) {
        union { uint8_t bytes[4]; float value; } phData;
        phData.bytes[0] = rx_data3[6];
        phData.bytes[1] = rx_data3[5];
        phData.bytes[2] = rx_data3[4];
        phData.bytes[3] = rx_data3[3];
        ph_read = phData.value;

        if(ph_read >= 0.0f && ph_read <= 14.0f) {
            ph_current = ph_read;
        } else {
            printf("pH parsed value abnormal (%.2f), skipping update\r\n", ph_read);
            return;
        }
        printf("Get_pH: %.2f (SetPoint: %.2f)\r\n", ph_current, ph_set);
    } else {
        printf("Invalid pH message!0x%02X 0x%02X 0x%02X\r\n", rx_data3[0], rx_data3[1], rx_data3[2]);
    }
}

// ========== 调整 pH 并控制泵 ==========
void adjustPH(void) {
    uint32_t now = HAL_GetTick();

    // 如果泵正在运行，检查是否到达运行时间
    if (pump_running) {
        if (now - pump_start_time >= PUMP_RUNTIME) {
            Stop_PUMP();               // 停止泵
            pump_running = 0;          // 更新状态
            pump_stop_time = now;      // 记录停止时间
            printf("Pump_STOP\n");
        }
        return; // 正在运行时，不再判断 pH
    }

    // 冷却时间内，不允许再次启动
    if (now - pump_stop_time < PUMP_COOLDOWN) {
        return;
    }

    // 检查 pH 偏差，决定是否启动泵
    if (ph_current < (ph_set - deadband)) {
        Start_PUMP();                 // 这里应该改成 Start_AcidPump()
        pump_running = 1;
        pump_start_time = now;
        printf("Acid_PUMP_START\n");
    } else if (ph_current > (ph_set + deadband)) {
        Start_PUMP();                 // 这里应该改成 Start_AlkaliPump()
        pump_running = 1;
        pump_start_time = now;
        printf("Alkali_PUMP_START\n");
    }
}

// ========== pH 任务主函数 ==========
void Task_PH(void) {
    requestPH();

    uint32_t timeout = HAL_GetTick();
    while(!rx_ph_flag && (HAL_GetTick() - timeout) < 100) {}

    if(rx_ph_flag) {
        readPH();
        HAL_UARTEx_ReceiveToIdle_DMA(&huart3, rx_data3, sizeof(rx_data3));
        __HAL_DMA_DISABLE_IT(huart3.hdmarx, DMA_IT_HT);
        rx_ph_flag = 0;
    } else {
        printf("pH receive timeout\r\n");
    }

    adjustPH();
}

