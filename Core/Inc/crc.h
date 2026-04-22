/*
 * crc.h
 *
 *  Created on: Mar 3, 2026
 *      Author: PIGflyme
 */

#ifndef INC_CRC_H_
#define INC_CRC_H_

#include <stdint.h>

uint16_t ModbusCRC16(uint8_t *buf, uint16_t len);

#endif /* INC_CRC_H_ */
