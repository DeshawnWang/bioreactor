/*
 * ads8688.h
 * Created on: Oct 17, 2025
 * Author: Tang
 */


#ifndef INC_ADS8688_H_
#define INC_ADS8688_H_

#include "spi.h"
#include "stdio.h"
#include <stdint.h>


// 引脚定义
#define ADS8688_CS_LOW()    HAL_GPIO_WritePin(GPIOE, GPIO_PIN_4, GPIO_PIN_RESET)
#define ADS8688_CS_HIGH()   HAL_GPIO_WritePin(GPIOE, GPIO_PIN_4, GPIO_PIN_SET)
#define ADS8688_RST_LOW()   HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_RESET)
#define ADS8688_RST_HIGH()  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_SET)
#define ADS8688_DAISY_LOW() HAL_GPIO_WritePin(GPIOE, GPIO_PIN_3, GPIO_PIN_RESET)

// 命令定义
#define NO_OP          0x0000
#define RST            0x8500
#define MAN_CH_0       0xC000
#define MAN_CH_1       0xC400
#define MAN_CH_2       0xC800
#define MAN_CH_3       0xCC00
#define MAN_CH_4       0xD000
#define MAN_CH_5       0xD400
#define MAN_CH_6       0xD800
#define MAN_CH_7       0xDC00

// 寄存器地址
#define AUTO_SEQ_EN                0x01
#define CH_PWR_DN                  0x02
#define CH0_INPUT_RANGE            0x05
#define CH1_INPUT_RANGE            0x06
#define CH2_INPUT_RANGE            0x07
#define CH3_INPUT_RANGE            0x08
#define CH4_INPUT_RANGE            0x09
#define CH5_INPUT_RANGE            0x0A
#define CH6_INPUT_RANGE            0x0B
#define CH7_INPUT_RANGE            0x0C
#define CH2_PGA            0x10   // 自定义逻辑位置，无硬件对应寄存器，只是方便调用

// 输入范围设置
#define VREF_B_125                 0x01  // 双极±5.12V
#define VREF_B_0625							0x02	// Í¨µÀÊäÈë·¶Î§¡À0.625*VREF
#define VREF_U_25								0x05	// 单极2.5*VREF
#define VREF_U_125							0x06	// Í¨µÀÊäÈë·¶Î§1.25*VREF
// 函数声明
void ADS8688_Init(void);
void ADS8688_Write_Command(uint16_t com);
void ADS8688_Write_Program(uint8_t addr, uint8_t data);
void Get_MAN_CH_Data(uint16_t ch, uint16_t *data);
void ADS8688_ReadOxygen(uint8_t ch);
void ADS8688_ReadCH2Voltage(void);



#endif /* INC_ADS8688_H_ */
