#include "relay.h"


void Relay_Init(void)
{
	Relay_Off();
}

void Relay_On(void) {
    HAL_GPIO_WritePin(RELAY_GPIO_Port, RELAY_Pin, GPIO_PIN_SET); // 低电平激活
    printf("[Relay] On (Heating)\r\n"); // 新增打印
}

void Relay_Off(void) {
    HAL_GPIO_WritePin(RELAY_GPIO_Port, RELAY_Pin, GPIO_PIN_RESET);   // 高电平关闭
    printf("[Relay] Off (Stop Heating)\r\n"); // 新增打印
}

void Relay_Switch(uint8_t onoff) {
    if (onoff) {
        Relay_On();
        // 可选：添加调试打印，确认函数被调用
        printf("[Relay] Switch to ON\r\n");
    } else {
        Relay_Off();
        printf("[Relay] Switch to OFF\r\n");
    }
}

