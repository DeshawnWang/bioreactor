/*
 * crc.c
 *
 *  Created on: Mar 3, 2026
 *      Author: PIGflyme
 */
#include "crc.h"

// 计算Modbus CRC16，返回值为CRC（低字节在前）
uint16_t ModbusCRC16(uint8_t *buf, uint16_t len)
{
    uint16_t crc = 0xFFFF;
    for (uint16_t i = 0; i < len; i++)
    {
        crc ^= buf[i];
        for (uint8_t j = 0; j < 8; j++)
        {
            if (crc & 1)
                crc = (crc >> 1) ^ 0xA001;
            else
                crc >>= 1;
        }
    }
    return crc;
}
