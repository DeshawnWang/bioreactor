#include "flower.h"

float current_flow3 = 0.0f;
float target_flow3 = 0.0f;

// ====================== 读取 03 号大流量计 ======================
void Flower_Read(void)
{
    uint8_t txbuf[8] = {0x03, 0x03, 0x00, 0x16, 0x00, 0x04, 0x00, 0x00};

    uint16_t crc = ModbusCRC16(txbuf, 6);      // 使用 stir.c 中定义的 CRC16
    txbuf[6] = crc & 0xFF;
    txbuf[7] = (crc >> 8) & 0xFF;

    HAL_UART_Transmit(&huart7, txbuf, 8, 100);
}

// ====================== 设置目标流量（上电后调用一次） ======================
void Flower_SetTarget(float target_lpm)
{
    int32_t raw = (int32_t)(target_lpm * 10000.0f);

    uint8_t txbuf[13];
    txbuf[0] = 0x03;
    txbuf[1] = 0x10;
    txbuf[2] = 0x00;
    txbuf[3] = 0x18;
    txbuf[4] = 0x00;
    txbuf[5] = 0x02;
    txbuf[6] = 0x04;
    txbuf[7] = (raw >> 24) & 0xFF;
    txbuf[8] = (raw >> 16) & 0xFF;
    txbuf[9] = (raw >> 8)  & 0xFF;
    txbuf[10]= raw & 0xFF;

    uint16_t crc = ModbusCRC16(txbuf, 11);
    txbuf[11] = crc & 0xFF;
    txbuf[12] = (crc >> 8) & 0xFF;

    HAL_UART_Transmit(&huart7, txbuf, 13, 100);
}

// ====================== 解析 03 号流量计返回数据 ======================
void Flower_Parse(uint8_t *buf)
{
    if (buf[0] != 0x03 || buf[1] != 0x03 || buf[2] != 0x08) return;

    uint8_t *p = &buf[3];

    uint32_t temp_current = ((uint32_t)p[2] << 24) | ((uint32_t)p[3] << 16) |
                            ((uint32_t)p[0] << 8) | p[1];
    uint32_t temp_target  = ((uint32_t)p[6] << 24) | ((uint32_t)p[7] << 16) |
                            ((uint32_t)p[4] << 8) | p[5];

    current_flow3 = (int32_t)temp_current * 0.0001f;
    target_flow3  = (int32_t)temp_target  * 0.0001f;

    printf("【03 大流量计】Current: %.4f LPM   Target: %.4f LPM\r\n",
           current_flow3, target_flow3);
}

// ====================== 流量计任务（简化，只读03） ======================
void Task_Flow(void)
{
    static uint32_t last_send_tick = 0;

    if (HAL_GetTick() - last_send_tick >= 800)
    {
        // 发送前清标志
        rx_stir_flag = 0;
        Flower_Read();
        last_send_tick = HAL_GetTick();

        // 等待一段时间（例如 200ms）
        uint32_t start = HAL_GetTick();
        while ((HAL_GetTick() - start) < 200) {
            if (rx_stir_flag) break;
        }

        if (rx_stir_flag) {
            // 打印所有接收到的原始数据，无论长度
            printf("RX7 raw: ");
            for (int i = 0; i < 20; i++) {
                printf("%02X ", rx_data7[i]);
            }
            printf("\r\n");

            // 正常解析
            Flower_Parse(rx_data7);
            rx_stir_flag = 0;
        } else {
            printf("No response from flow meter\r\n");
        }

        // 重启接收
        HAL_UARTEx_ReceiveToIdle_DMA(&huart7, rx_data7, sizeof(rx_data7));
        __HAL_DMA_DISABLE_IT(huart7.hdmarx, DMA_IT_HT);
    }
}
