#include "pid.h"

// 防积分饱和的限制
static float clamp(float val, float min, float max) {
    if (val > max) return max;
    if (val < min) return min;
    return val;
}

void PID_Init(PID_t *pid, float Kp, float Ki, float Kd, float out_min, float out_max) {
    pid->Kp = Kp;
    pid->Ki = Ki;
    pid->Kd = Kd;
    pid->prev_error = 0;
    pid->integral = 0;
    pid->out_min = out_min;
    pid->out_max = out_max;
    pid->derivative = 0;
    pid->last_output = 0;
}

// 主 PID 任务
float PID_Task(PID_t *pid, float setpoint, float measurement, float dt) {
    float error = setpoint - measurement;

    // 积分项（带限幅，防积分饱和）
    pid->integral += error * dt;
    pid->integral = clamp(pid->integral, pid->out_min, pid->out_max);

    // 微分项（带滤波）
    float raw_derivative = (error - pid->prev_error) / dt;
    const float alpha = 0.7f;  // 低通滤波系数（0~1，越小越平滑）
    pid->derivative = alpha * pid->derivative + (1 - alpha) * raw_derivative;

    // PID 输出
    float output = pid->Kp * error + pid->Ki * pid->integral + pid->Kd * pid->derivative;

    // 输出限幅
    output = clamp(output, pid->out_min, pid->out_max);

    pid->prev_error = error;
    pid->last_output = output;

    // 调试信息打印
    printf("[PID] SP=%.2f, PV=%.2f, Err=%.2f, Out=%.2f, P=%.2f, I=%.2f, D=%.2f\r\n",
           setpoint, measurement, error, output,
           pid->Kp * error,
           pid->Ki * pid->integral,
           pid->Kd * pid->derivative);

    return output;
}

// 自整定（Relay Auto-tuning）
void PID_AutoTune(PID_Params_t *params,
                  float (*readTemp)(void),
                  void (*relayCtrl)(uint8_t),
                  float target_temp,
                  uint32_t duration_ms)
{
    uint32_t start_time = HAL_GetTick();
    uint32_t last_switch = start_time;
    uint32_t last_cross_time = 0;
    uint32_t period_accum = 0, period_count = 0;

    float temp, temp_max = -1000.0f, temp_min = 1000.0f;
    float last_temp;
    uint8_t relay_state = 1; // 一开始就开继电器

    // 打开继电器，保证开始有动作
    relayCtrl(1);
    last_temp = readTemp();

    printf("=== PID AutoTune Start ===\r\n");

    while (HAL_GetTick() - start_time < duration_ms)
    {
        temp = readTemp();

        // 新增：判断温度是否异常（避免ADC错误导致逻辑紊乱）
        if (temp <= -50.0f) {  // 错误温度值（低于合理范围）
            printf("[AutoTune] Temp Error: %.2f\r\n", temp);
            HAL_Delay(500);
            continue;  // 跳过本次循环，避免异常计算
        }

        // 更新温度极值
        if (temp > temp_max) temp_max = temp;
        if (temp < temp_min) temp_min = temp;

        // 过中值检测，计算振荡周期（仅当温度范围有效时）
        if (temp_max - temp_min > 0.5f) {  // 温度变化超过0.5℃才计算中值，避免噪声干扰
            float mid = (temp_max + temp_min) / 2.0f;
            if ((last_temp < mid && temp >= mid) || (last_temp > mid && temp <= mid))
            {
                if (last_cross_time != 0)
                {
                    period_accum += HAL_GetTick() - last_cross_time;
                    period_count++;
                }
                last_cross_time = HAL_GetTick();
            }
        }
        last_temp = temp;

        // 定时翻转继电器，每5秒翻转一次
        if (HAL_GetTick() - last_switch >= 5000)
        {
            relay_state = !relay_state;
            relayCtrl(relay_state);
            last_switch = HAL_GetTick();

            printf("[AutoTune] Relay=%s, Temp=%.2f, Max=%.2f, Min=%.2f\r\n",
                   relay_state ? "ON" : "OFF", temp, temp_max, temp_min);
        }

        HAL_Delay(20); // 小延时，保证系统响应
    }

    // 关闭继电器
    relayCtrl(0);

    // 计算Ku和Tu（增加period_count判断，避免除零）
    float A = (temp_max - temp_min) / 2.0f;  // 温度振幅
    float a = 1.0f;                          // 继电器输出幅值假设为1
    float Ku = 0.0f, Tu = 1.0f;
    if (A > 0.1f) {  // 温度振幅有效才计算
        Ku = (4.0f * a) / (3.14159f * A);  // 临界增益
    }
    if (period_count > 0) {
        Tu = (float)period_accum / period_count / 1000.0f;  // 振荡周期（秒）
    }

    // Ziegler–Nichols 调参（PID）
    params->Kp = 0.6f * Ku;
    params->Ki = 2.0f * params->Kp / Tu;   // Ki = 2*Kp/Tu
    params->Kd = params->Kp * Tu / 8.0f;  // Kd = Kp*Tu/8

    printf("=== AutoTune Done ===\r\n");
    printf("Temp range: [%.2f, %.2f] C\r\n", temp_min, temp_max);
    printf("Ku=%.3f, Tu=%.3f sec\r\n", Ku, Tu);
    printf("Kp=%.3f, Ki=%.3f, Kd=%.3f\r\n", params->Kp, params->Ki, params->Kd);
}
