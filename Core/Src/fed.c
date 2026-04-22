#include "fed.h"//补料系统 3泵

void Start_FedPump(){
	uint8_t sendBuffer[] = {
	        0x03, 0x10, 0x00, 0x60, 0x00, 0x04, 0x08,
	        0x00, 0x00, 0x0B, 0xB8, 0x00, 0x64, 0x00, 0x64, 0x55, 0xAF
	};


	tx_pump_flag = 0;
	    HAL_UART_Transmit_DMA(&huart6, sendBuffer, sizeof(sendBuffer));
	    while (tx_pump_flag == 0) {}  // 等待发送完成
	    // 开启接收
	    HAL_UARTEx_ReceiveToIdle_DMA(&huart6, rx_data6, sizeof(rx_data6));
	    __HAL_DMA_DISABLE_IT(huart6.hdmarx, DMA_IT_HT);
}

void Stop_FedPump(void)
{
    uint8_t sendBuffer[] = {
        0x03, 0x10, 0x00, 0x03, 0x00, 0x02,
        0x04, 0x00, 0x00, 0x00, 0x00, 0xB8, 0x02
    };

    tx_pump_flag = 0;
    HAL_UART_Transmit_DMA(&huart6, sendBuffer, sizeof(sendBuffer));
    while (tx_pump_flag == 0) {}  // 等待发送完成
    // 开启接收
    HAL_UARTEx_ReceiveToIdle_DMA(&huart6, rx_data6, sizeof(rx_data6));
    __HAL_DMA_DISABLE_IT(huart6.hdmarx, DMA_IT_HT);
}

void Fed_Judge(){

}
