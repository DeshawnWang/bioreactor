#include "pwm2.h"

extern float DO_mgL;
float target_oxygen = 10.0f;     // 目标溶氧值
uint16_t pwm_duty = 0;          // PWM占空比 (0~100)
uint16_t pwm_max = 100;         // 最大占空比100%
uint16_t pwm_min = 10;          // 最小启动占空比10%

// 将百分比转换为CCR值
void Set_PWM_Duty(float duty_percent)
{
    if (duty_percent > 100) duty_percent = 100;
    if (duty_percent < 0)   duty_percent = 0;

    uint16_t ccr = (uint16_t)(duty_percent / 100.0f *
                              (__HAL_TIM_GET_AUTORELOAD(&htim1) + 1));
    __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_4, ccr);
}

void Pump_Control_Update(void)
{
    if (DO_mgL < target_oxygen - 0.1f) {
        pwm_duty += 5;
        if (pwm_duty > pwm_max) pwm_duty = pwm_max;
    } else if (DO_mgL > target_oxygen + 0.3f) {
        pwm_duty -= 5;
        if (pwm_duty < pwm_min) pwm_duty = pwm_min;
    }

    Set_PWM_Duty(pwm_duty);
    printf("Pump PWM = %d%% (DO=%.2f)\r\n", pwm_duty, DO_mgL);
}

void PWM2_Init(void)
{
    HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_4);
    __HAL_TIM_MOE_ENABLE(&htim1);  // ✅ 开启主输出
    Set_PWM_Duty(30);
    printf("PWM2 init OK: PE14 -> TIM1_CH4 10kHz\r\n");
}
