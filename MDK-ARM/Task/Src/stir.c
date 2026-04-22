#include "stir.h"  //uart7

uint8_t rx_data7[RX_BUFFER_SIZE];    // uart7接收缓存
 uint8_t tx_stir_flag = 0;    // uart7发送完成标志
 uint8_t rx_stir_flag = 0;
 //发送使能信号
 void Get_Sign(void){
	 uint8_t sendBuffer[] = {0x01, 0x06, 0x00, 0x00, 0x00, 0x01, 0x48, 0x0A};
	 tx_stir_flag = 0;
     HAL_UART_Transmit_DMA(&huart7, sendBuffer, sizeof(sendBuffer));
     while (tx_stir_flag == 0) {}  // 等待发送完成
         // 开启接收
     HAL_UARTEx_ReceiveToIdle_DMA(&huart7, rx_data7, sizeof(rx_data7));
     __HAL_DMA_DISABLE_IT(huart7.hdmarx, DMA_IT_HT);
 }
 //启动速度模式
 void SpeedMode(void){
 	uint8_t sendBuffer[] = {0x01, 0x06, 0x00, 0x19, 0x00, 0x03, 0x18, 0x0C};
 	tx_stir_flag = 0;
 	HAL_UART_Transmit_DMA(&huart7, sendBuffer, sizeof(sendBuffer));
 	while (tx_stir_flag == 0) {}  // 等待发送完成
 	         // 开启接收
 	         HAL_UARTEx_ReceiveToIdle_DMA(&huart7, rx_data7, sizeof(rx_data7));
 	         __HAL_DMA_DISABLE_IT(huart7.hdmarx, DMA_IT_HT);
 }

 //转速300
void Start_Stir(void){
	uint8_t sendBuffer[] = {0x01, 0x06, 0x00, 0x02, 0x01, 0x2C, 0x28, 0x47};
	tx_stir_flag = 0;
	HAL_UART_Transmit_DMA(&huart7, sendBuffer, sizeof(sendBuffer));
	while (tx_stir_flag == 0) {}  // 等待发送完成
	         // 开启接收
	         HAL_UARTEx_ReceiveToIdle_DMA(&huart7, rx_data7, sizeof(rx_data7));
	         __HAL_DMA_DISABLE_IT(huart7.hdmarx, DMA_IT_HT);
}
//转速0
void Stop_Stir(void){
	uint8_t sendBuffer[] = {0x01, 0x06, 0x00, 0x02, 0x00, 0x00, 0x28, 0x0A};
	tx_stir_flag = 0;
	HAL_UART_Transmit_DMA(&huart7, sendBuffer, sizeof(sendBuffer));
	while (tx_stir_flag == 0) {}  // 等待发送完成
	         // 开启接收
	         HAL_UARTEx_ReceiveToIdle_DMA(&huart7, rx_data7, sizeof(rx_data7));
	         __HAL_DMA_DISABLE_IT(huart7.hdmarx, DMA_IT_HT);
}


