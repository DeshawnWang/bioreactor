#include "ads8688.h"

// ================= DO校准参数 =================
#define DO_ZERO_CAL 0.65f      // 亚硫酸钠测得 DO_raw
#define DO_SPAN_CAL 10.73f     // 最大通气 DO_raw

// ================= 滤波参数 =================
#define DO_FILTER_N 5

static float do_buffer[DO_FILTER_N] = {0};
static uint8_t do_index = 0;

float DO_mgL = 0.0f;  // 全局变量，存储溶氧值

extern SPI_HandleTypeDef hspi4;
//滤波函数
static float DO_Filter(float new_val)
{
    do_buffer[do_index++] = new_val;

    if(do_index >= DO_FILTER_N)
        do_index = 0;

    float sum = 0.0f;

    for(int i = 0; i < DO_FILTER_N; i++)
        sum += do_buffer[i];

    return sum / DO_FILTER_N;
}

// 写命令函数
void ADS8688_Write_Command(uint16_t com)
{
    uint8_t wr_data[2];
    wr_data[0] = (uint8_t)(com >> 8);
    wr_data[1] = (uint8_t)(com & 0xFF);

    ADS8688_CS_LOW();
    HAL_SPI_Transmit(&hspi4, wr_data, 2, HAL_MAX_DELAY);
    ADS8688_CS_HIGH();
}

// 写寄存器函数
void ADS8688_Write_Program(uint8_t addr, uint8_t data)
{
    uint8_t wr_data[2];
    wr_data[0] = (addr << 1) | 0x01;
    wr_data[1] = data;

    ADS8688_CS_LOW();
    HAL_SPI_Transmit(&hspi4, wr_data, 2, HAL_MAX_DELAY);
    ADS8688_CS_HIGH();
}

// 初始化函数（全部通道 0~10.24V）
void ADS8688_Init(void)
{
    ADS8688_DAISY_LOW();

    // 硬件复位
    ADS8688_RST_LOW();
    HAL_Delay(2);
    ADS8688_RST_HIGH();
    HAL_Delay(2);

    // 软件复位
    ADS8688_Write_Command(RST);
    HAL_Delay(2);

    // 所有通道：单极 0–10.24V
    ADS8688_Write_Program(CH0_INPUT_RANGE, VREF_U_25);
    ADS8688_Write_Program(CH1_INPUT_RANGE, VREF_U_25);
    ADS8688_Write_Program(CH2_INPUT_RANGE, VREF_U_25);
    ADS8688_Write_Program(CH3_INPUT_RANGE, VREF_U_25);
    ADS8688_Write_Program(CH4_INPUT_RANGE, VREF_U_25);
    ADS8688_Write_Program(CH5_INPUT_RANGE, VREF_U_25);
    ADS8688_Write_Program(CH6_INPUT_RANGE, VREF_U_25);
    ADS8688_Write_Program(CH7_INPUT_RANGE, VREF_U_25);

    // 启用全部通道
    ADS8688_Write_Program(CH_PWR_DN, 0x00);
    ADS8688_Write_Program(AUTO_SEQ_EN, 0xFF);

    // 选择通道 0 开始
    ADS8688_Write_Command(MAN_CH_0);

    printf("ADS8688 Initialized:  CH0~7 \r\n");
}

// 读取单通道原始值
void Get_MAN_CH_Data(uint16_t ch, uint16_t *data)
{
    uint8_t Tx[4] = {0}, Rx[4] = {0};

    ADS8688_Write_Command(ch);
    for (volatile int i = 0; i < 10; i++);  // 简单延时

    ADS8688_CS_LOW();
    HAL_SPI_TransmitReceive(&hspi4, Tx, Rx, 4, HAL_MAX_DELAY);
    ADS8688_CS_HIGH();

    *data = ((uint16_t)Rx[2] << 8) | Rx[3];
}

// 读取指定通道并换算为 mA 与 DO(%)
void ADS8688_ReadOxygen(uint8_t ch)
{
    if (ch > 7) return; // 防止越界

    const uint16_t channels[8] = {
        MAN_CH_0, MAN_CH_1, MAN_CH_2, MAN_CH_3,
        MAN_CH_4, MAN_CH_5, MAN_CH_6, MAN_CH_7
    };

    const char *names[8] = {
        "CH0","CH1","CH2","CH3","CH4","CH5","CH6","CH7"
    };

    // ==== ADC参数 ====
    const float V_ZERO = 1.996f;   // 4mA电压
    const float V_FULL = 9.980f;   // 20mA电压
    const float V_FS   = 10.24f;   // ADC满量程
    const float ADC_FULL = 65536.0f;

    uint16_t adc_data;

    Get_MAN_CH_Data(channels[ch], &adc_data);

    // 1. ADC → 电压
    float voltage = (float)adc_data * V_FS / ADC_FULL;

    // 2. 电压 → 电流 (mA)
    float current_mA = (voltage - V_ZERO) / (V_FULL - V_ZERO) * 16.0f + 4.0f;

    // 3. 电流 → DO_raw (mg/L)
    float DO_raw = (current_mA - 4.0f) * 20.0f / 16.0f;

    // 4. 两点校准 → DO%
    float DO_percent = (DO_raw - DO_ZERO_CAL) * 100.0f /
                       (DO_SPAN_CAL - DO_ZERO_CAL);

    // 5. 滤波
    DO_percent = DO_Filter(DO_percent);

    // 6. 保存
    DO_mgL = DO_percent;

    // 7. 打印调试信息
    printf("%s: %7.3f V | %6.3f mA | DO_raw=%6.2f mg/L | DO=%6.2f %% | adc=0x%04X\r\n",
           names[ch], voltage, current_mA, DO_raw, DO_mgL, adc_data);
}


void ADS8688_Readantifoam(uint8_t ch){

}
