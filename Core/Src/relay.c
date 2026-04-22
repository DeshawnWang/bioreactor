#include "relay.h"

static uint8_t relay_status = 0;  // 记录继电器状态

void Relay_Init(void)
{
	Relay_Off();
    relay_status = 0;
}

void Relay_On(void) {
    HAL_GPIO_WritePin(RELAY_GPIO_Port, RELAY_Pin, GPIO_PIN_SET); // 高电平激活
    relay_status = 1;
    printf("[Relay] On (Heating)\r\n");
}

void Relay_Off(void) {
    HAL_GPIO_WritePin(RELAY_GPIO_Port, RELAY_Pin, GPIO_PIN_RESET);   // 低电平关闭
    relay_status = 0;
    printf("[Relay] Off (Stop Heating)\r\n");
}

void Relay_Switch(uint8_t onoff) {
    if (onoff && !relay_status) {
        Relay_On();
    } else if (!onoff && relay_status) {
        Relay_Off();
    }
}

uint8_t Get_Relay_Status(void) {
    return relay_status;
}
