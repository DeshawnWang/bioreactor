#include "endgas.h"

uint8_t rx_data4[RX_BUFFER_SIZE];    // uart4接收缓存
 uint8_t tx_en_flag = 0;    // uart4发送完成标志
 uint8_t rx_en_flag = 0;
void Get_Endgas(void){
    static const uint8_t cmd_Endgas[8] = {0x06, 0x03, 0x00, 0x01, 0x00, 0x08, 0x14, 0x7B};
    memset(rx_data4, 0, sizeof(rx_data4));

    tx_en_flag = 0;
    HAL_UART_Transmit_DMA(&huart4, (uint8_t*)cmd_Endgas, sizeof(cmd_Endgas));
    while (tx_en_flag == 0);  // 等待发送完成再启动接收

    HAL_UARTEx_ReceiveToIdle_DMA(&huart4, rx_data4, sizeof(rx_data4));
}
 void Read_Endgas(void) {


     uint16_t len = RX_BUFFER_SIZE;
     uint8_t found = 0;
     // 调试打印：查看当前接收缓冲区的实际数据
         printf("endgas dump: ");
         for (uint16_t k = 0; k < 40; k++) {  // 打印前40字节够用了
             printf("%02X ", rx_data4[k]);
         }
         printf("\n");
     // 遍历缓冲区查找报文头 06 03 10 00 06
     for (uint16_t i = 0; i < len - 12; i++) { // 后续至少要有12字节数据
         if (rx_data4[i] == 0x06 &&
             rx_data4[i+1] == 0x03 &&
             rx_data4[i+2] == 0x10 &&
             rx_data4[i+3] == 0x00 &&
             rx_data4[i+4] == 0x06)
         {
             found = 1;
             printf("找到尾气报文头，位置: %d\n", i);

             // 打印报文头后的完整报文（12字节数据 + CRC）
             printf("尾气原始报文: ");
             for (uint16_t j = i; j < i + 13; j++) {
                 printf("%02X ", rx_data4[j]);
             }
             printf("\n");

             // 只解析原本的三个数据
             uint16_t temp_raw = ((uint16_t)rx_data4[i+5] << 8) | rx_data4[i+6];
             uint16_t co2_raw  = ((uint16_t)rx_data4[i+9] << 8) | rx_data4[i+10];
             uint16_t o2_raw   = ((uint16_t)rx_data4[i+11] << 8) | rx_data4[i+12];

             float temp_celsius = (float)temp_raw / 10.0f;
             float o2_percent   = (float)o2_raw / 100.0f;

             printf("解析后的尾气数据:\n");
             printf("gas_temp: %.1f °C\n", temp_celsius);
             printf("gas_CO₂: %d ppm\n", co2_raw);
             printf("gas_O₂ : %.2f %%\n", o2_percent);

             break; // 找到第一个报文后退出
         }
     }

     if (!found) {
         printf("未找到有效尾气报文头\n");
     }
 }

 void Endgas_Task(void){
     Get_Endgas();

     // 等待数据到达（最多200ms）
     uint32_t start = HAL_GetTick();
     while (!rx_en_flag && (HAL_GetTick() - start < 200)) {}

     if (rx_en_flag) {
         Read_Endgas();
         rx_en_flag = 0;
     } else {
         printf("尾气接收超时\n");
     }
 }
