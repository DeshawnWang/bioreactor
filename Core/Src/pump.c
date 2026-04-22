#include "pump.h"
//usart6
uint8_t rx_data6[RX_BUFFER_SIZE];
uint8_t tx_pump_flag = 0;
uint8_t rx_pump_flag = 0;


// ================= 启动泵 =================
void Start_AcidPump(void)//地址是01
{
    uint8_t sendBuffer[] = {
        0x01, 0x10, 0x00, 0x60, 0x00, 0x04, 0x08,
        0x00, 0x00, 0x0B, 0xB8, 0x01, 0x2C, 0x00, 0x64, 0x56, 0x44
    };

    tx_pump_flag = 0;
    HAL_UART_Transmit_DMA(&huart6, sendBuffer, sizeof(sendBuffer));
    while (tx_pump_flag == 0) {}  // 等待发送完成
    // 开启接收
    HAL_UARTEx_ReceiveToIdle_DMA(&huart6, rx_data6, sizeof(rx_data6));
    __HAL_DMA_DISABLE_IT(huart6.hdmarx, DMA_IT_HT);
}
void Start_AlkaliPump(void)//地址是02
{
    uint8_t sendBuffer[] = {
        0x02, 0x10, 0x00, 0x60, 0x00, 0x04, 0x08,
        0x00, 0x00, 0x0B, 0xB8, 0x01, 0x2C, 0x00, 0x64, 0x15, 0x45
    };

    tx_pump_flag = 0;
    HAL_UART_Transmit_DMA(&huart6, sendBuffer, sizeof(sendBuffer));
    while (tx_pump_flag == 0) {}  // 等待发送完成

    // 开启接收
    HAL_UARTEx_ReceiveToIdle_DMA(&huart6, rx_data6, sizeof(rx_data6));
    __HAL_DMA_DISABLE_IT(huart6.hdmarx, DMA_IT_HT);
}

// ================= 停止泵 =================
void Stop_AcidPump(void)//01
{
    uint8_t sendBuffer[] = {
        0x01, 0x10, 0x00, 0x03, 0x00, 0x02,
        0x04, 0x00, 0x00, 0x00, 0x00, 0xB3, 0xBA
    };

    tx_pump_flag = 0;
    HAL_UART_Transmit_DMA(&huart6, sendBuffer, sizeof(sendBuffer));
    while (tx_pump_flag == 0) {}  // 等待发送完成
    // 开启接收
    HAL_UARTEx_ReceiveToIdle_DMA(&huart6, rx_data6, sizeof(rx_data6));
    __HAL_DMA_DISABLE_IT(huart6.hdmarx, DMA_IT_HT);
}
void Stop_AlkaliPump(void)//02
{
    uint8_t sendBuffer[] = {
        0x02, 0x10, 0x00, 0x03, 0x00, 0x02,
        0x04, 0x00, 0x00, 0x00, 0x00, 0xBC, 0xFE
    };
    tx_pump_flag = 0;
    HAL_UART_Transmit_DMA(&huart6, sendBuffer, sizeof(sendBuffer));
    while (tx_pump_flag == 0) {}  // 等待发送完成
    // 开启接收
    HAL_UARTEx_ReceiveToIdle_DMA(&huart6, rx_data6, sizeof(rx_data6));
    __HAL_DMA_DISABLE_IT(huart6.hdmarx, DMA_IT_HT);
}


