#include "stir.h"  //uart7白A黑B


uint8_t rx_data7[RX_BUFFER_SIZE];    // uart7接收缓存
 uint8_t tx_stir_flag = 0;    // uart7发送完成标志
 uint8_t rx_stir_flag = 0;


 //发送使能信号
 void Get_Sign(){
	 uint8_t sendBuffer[] = {0x01, 0x06, 0x00, 0x00, 0x00, 0x01, 0x48, 0x0A};
	 tx_stir_flag = 0;
     HAL_UART_Transmit_DMA(&huart7, sendBuffer, sizeof(sendBuffer));
     while (tx_stir_flag == 0) {}  // 等待发送完成
         // 开启接收
     HAL_UARTEx_ReceiveToIdle_DMA(&huart7, rx_data7, sizeof(rx_data7));
     __HAL_DMA_DISABLE_IT(huart7.hdmarx, DMA_IT_HT);

 }
 //启动速度模式
 void SpeedMode(){
 	uint8_t sendBuffer[] = {0x01, 0x06, 0x00, 0x19, 0x00, 0x03, 0x18, 0x0C};
 	tx_stir_flag = 0;
 	HAL_UART_Transmit_DMA(&huart7, sendBuffer, sizeof(sendBuffer));
 	while (tx_stir_flag == 0) {}  // 等待发送完成
 	         // 开启接收
 	         HAL_UARTEx_ReceiveToIdle_DMA(&huart7, rx_data7, sizeof(rx_data7));
 	         __HAL_DMA_DISABLE_IT(huart7.hdmarx, DMA_IT_HT);

 }

 /**
  * @brief  设置搅拌电机转速（带方向）
  * @param  speed: 转速值，范围 -1000 ~ 1000
  *               正值为逆时针，负值为顺时针
  * @retval None
  */
//设置速度
 void Set_Stir_Speed(int16_t speed)
 {
     // 限制输入范围
     if (speed > 1000) speed = 1000;
     if (speed < -1000) speed = -1000;

     uint8_t sendBuffer[8];
     sendBuffer[0] = 0x01;                     // 设备地址
     sendBuffer[1] = 0x06;                     // 功能码：写单个寄存器
     sendBuffer[2] = 0x00;                     // 寄存器地址高字节 (0x02)
     sendBuffer[3] = 0x02;                     // 寄存器地址低字节
     sendBuffer[4] = (uint8_t)((speed >> 8) & 0xFF); // 数据高字节
     sendBuffer[5] = (uint8_t)(speed & 0xFF);        // 数据低字节

     uint16_t crc = ModbusCRC16(sendBuffer, 6);
     sendBuffer[6] = crc & 0xFF;
     sendBuffer[7] = (crc >> 8) & 0xFF;

     tx_stir_flag = 0;
     HAL_UART_Transmit_DMA(&huart7, sendBuffer, 8);
     while (tx_stir_flag == 0) {}  // 等待发送完成

     // 重新开启接收（保持与现有代码一致）
     HAL_UARTEx_ReceiveToIdle_DMA(&huart7, rx_data7, sizeof(rx_data7));
     __HAL_DMA_DISABLE_IT(huart7.hdmarx, DMA_IT_HT);
 }

 /**
  * @brief  解析搅拌器二进制命令并执行
  * @param  buffer: 接收到的数据缓冲区
  * @param  size:   数据长度
  * 电脑发送到串口控制速度
  * @retval None
  */
 void Process_Stir_Command(uint8_t *buf, uint16_t len)
 {
	 printf("Recv Stir Cmd\r\n");
     /* 协议固定8字节 */
     if(len != 8) return;

     /* 判断帧头 */
     if(buf[0] != 0xAA) return;

     /* CRC校验 */
     uint16_t crc_calc = ModbusCRC16(buf,6);
     uint16_t crc_recv = buf[6] | (buf[7] << 8);

     if(crc_calc != crc_recv)
     {
         printf("CRC error\r\n");
         return;
     }

     uint8_t addr = buf[1];
     uint8_t cmd  = buf[2];
     uint8_t type = buf[3];

     int16_t value = (buf[4] << 8) | buf[5];

     if(addr != 0x01) return;

     /* 写命令 */
     if(cmd == 0x06)
     {
         switch(type)
         {
             case 0x01:      // 速度
                 Set_Stir_Speed(value);
                 break;

//             case 0x02:      // 加速度
//                 Set_Stir_Acc(value);
//                 break;
         }

         /* 应答：原样返回 */
         HAL_UART_Transmit(&huart1, buf, len, 100);
     }
 }

 /**
  * @brief 读取搅拌电机当前转速（寄存器 0x0010）
  * @retval 实际转速（r/min），已除以10。读取失败返回 0x7FFFFFFF（大正数表示错误）
  */
 int32_t Read_Stir_Speed(void)
 {
     uint8_t txbuf[8] = {0x01, 0x03, 0x00, 0x10, 0x00, 0x01, 0x00, 0x00};

     uint16_t crc = ModbusCRC16(txbuf, 6);
     txbuf[6] = crc & 0xFF;
     txbuf[7] = (crc >> 8) & 0xFF;

     rx_stir_flag = 0;                    // 关键：清空旧标志

     tx_stir_flag = 0;
     HAL_UART_Transmit_DMA(&huart7, txbuf, 8);
     while (tx_stir_flag == 0);

     HAL_UARTEx_ReceiveToIdle_DMA(&huart7, rx_data7, sizeof(rx_data7));
     __HAL_DMA_DISABLE_IT(huart7.hdmarx, DMA_IT_HT);

     uint32_t timeout = HAL_GetTick();
     while (rx_stir_flag == 0 && (HAL_GetTick() - timeout) < 250);   // 稍微加长一点

     if (rx_stir_flag == 0)
     {
         printf("Read Stir Speed Timeout!\r\n");
         return 0x7FFFFFFF;
     }

     // ==================== 严格只认 0x01 的响应 ====================
     if (rx_data7[0] == 0x01 && rx_data7[1] == 0x03 && rx_data7[2] == 0x02)
     {
         uint16_t crc_recv = (rx_data7[6] << 8) | rx_data7[5];
         uint16_t crc_calc = ModbusCRC16(rx_data7, 5);

         if (crc_calc == crc_recv)
         {
             int16_t raw_speed = (int16_t)((rx_data7[3] << 8) | rx_data7[4]);
             int32_t real_speed = raw_speed / 10;

             printf("Stir Motor Speed: %ld r/min (raw=0x%04X)\r\n", real_speed, (uint16_t)raw_speed);

             rx_stir_flag = 0;
             return real_speed;
         }
         else
         {
             printf("Stir Speed CRC Error!\r\n");
         }
     }
     else
     {
         printf("Stir Speed Response Error! Head=0x%02X Len=%d\r\n", rx_data7[0], rx_data7[2]);
         // 打印前10个字节帮助调试
         printf("Data: ");
         for (int i = 0; i < 10; i++) printf("%02X ", rx_data7[i]);
         printf("\r\n");
     }

     rx_stir_flag = 0;
     return 0x7FFFFFFF;
 }

