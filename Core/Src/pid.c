#include "pid.h"
#include "stdio.h"
#include <math.h>

// ================= PID 初始化 =================
void PID_Init(PID_HandleTypeDef *pid, float Kp, float Ki, float Kd, float min, float max) {
    pid->params.Kp = Kp;
    pid->params.Ki = Ki;
    pid->params.Kd = Kd;
    pid->setpoint = 0.0f;
    pid->last_error = 0.0f;
    pid->integral = 0.0f;
    pid->output_min = min;
    pid->output_max = max;
    pid->output = 0.0f;
    pid->last_pv = 0.0f;
}

// ================= PID 计算 =================
float PID_Calculate(PID_HandleTypeDef *pid, float setpoint, float pv, float dt) {
    pid->setpoint = setpoint;
    float error = setpoint - pv;

    // --- 改进的积分抗饱和 ---
    // 只在误差较小时积累积分，避免积分饱和
    if (fabsf(error) < 0.2f) {
        pid->integral += pid->params.Ki * error * dt;
    } else if (fabsf(error) < 0.5f) {
        pid->integral += pid->params.Ki * error * dt * 0.5f; // 减半积分
    } else {
        pid->integral *= 0.8f; // 大误差时快速衰减积分
    }

    // --- 更严格的积分限幅 ---
    float Imax = pid->output_max * 0.2f; // 减小积分上限
    if (pid->integral > Imax) pid->integral = Imax;
    if (pid->integral < -Imax) pid->integral = -Imax;

    // --- 改进的微分滤波 ---
    float derivative_raw = (error - pid->last_error) / dt;
    static float derivative_filt = 0.0f;
    float alpha = 0.3f; // 更强的滤波
    derivative_filt = alpha * derivative_raw + (1 - alpha) * derivative_filt;
    float derivative = derivative_filt;

    pid->last_error = error;

    // --- PID 输出计算 ---
    float output = pid->params.Kp * error + pid->integral + pid->params.Kd * derivative;

    // 输出限幅
    if (output > pid->output_max) output = pid->output_max;
    if (output < pid->output_min) output = pid->output_min;

    pid->output = output;

    return output;
}

// ================= 更新 PID 参数 =================
void PID_UpdateParams(PID_HandleTypeDef *pid, float Kp, float Ki, float Kd) {
    pid->params.Kp = Kp;
    pid->params.Ki = Ki;
    pid->params.Kd = Kd;
    pid->integral = 0;
    pid->last_error = 0;
}

// ================= 自适应微调（防止长期偏差） =================
void PID_SelfAdjust(PID_HandleTypeDef *pid, float error, float dTemp) {
    if (fabsf(error) < 0.3f && fabsf(dTemp) < 0.02f) {
        pid->params.Kp *= 0.999f;   // 逐步降低Kp防止过冲
    } else if (fabsf(error) > 0.5f) {
        pid->params.Kp *= 1.002f;   // 稍微增强响应
    }

    // 限制范围
    if (pid->params.Kp < 1.0f) pid->params.Kp = 1.0f;
    if (pid->params.Kp > 20.0f) pid->params.Kp = 20.0f;
}


void PID_AutoTune_Init(AutoTuneHandle *h, PID_Params_t *p, float (*readTemp)(void),
                      void (*relayCtrl)(uint8_t), float target, uint32_t duration) {
    h->state = TUNE_START;
    h->params = p;
    h->readTemp = readTemp;
    h->relayCtrl = relayCtrl;
    h->target = target;
    h->duration = duration;
    h->start_time = HAL_GetTick();
    h->last_switch = h->start_time;
    h->last_cross_time = 0;
    h->period_accum = 0;
    h->period_count = 0;
    h->temp_max = -1000.0f;
    h->temp_min = 1000.0f;
    h->last_temp = readTemp();
    h->relay_state = 1;
    relayCtrl(1);
    printf("=== PID AutoTune Start ===\r\n");
}

void PID_AutoTune_Task(AutoTuneHandle *h) {
    if (h->state == TUNE_IDLE) return;
    uint32_t now = HAL_GetTick();
    float temp = h->readTemp();
    if (temp <= -50.0f) return;

    // 记录最大最小温度
    if (temp > h->temp_max) h->temp_max = temp;
    if (temp < h->temp_min) h->temp_min = temp;

    float amplitude = (h->temp_max - h->temp_min) / 2.0f;

    // --- 计算振荡周期 ---
    float mid = (h->temp_max + h->temp_min) / 2.0f;
    if ((h->last_temp < mid && temp >= mid) || (h->last_temp > mid && temp <= mid)) {
        if (h->last_cross_time != 0) {
            h->period_accum += now - h->last_cross_time;
            h->period_count++;
        }
        h->last_cross_time = now;
    }

    // --- 自适应继电器切换 ---
    if (fabs(temp - h->target) > 0.3f) {
        // 温差大时加热时间长
        if (now - h->last_switch > 4000)
            h->relay_state = (temp < h->target);
    } else {
        // 温差小，减少扰动
        if (now - h->last_switch > 8000)
            h->relay_state = !h->relay_state;
    }
    h->relayCtrl(h->relay_state);
    h->last_switch = now;

    h->last_temp = temp;

    // --- 判断收敛 ---
    if (h->period_count >= 6) {
        float avg_period = (float)h->period_accum / h->period_count / 1000.0f;
        float Ku = (4.0f * 1.0f) / (3.14159f * amplitude);
        float Kp = 0.33f * Ku;
        float Ki = 2.0f * Kp / avg_period;
        float Kd = Kp * avg_period / 3.0f;

        // 如果振荡幅度连续3次下降，认为收敛
        static float prev_amp[3] = {0};
        prev_amp[0] = prev_amp[1];
        prev_amp[1] = prev_amp[2];
        prev_amp[2] = amplitude;

        if (prev_amp[0] > prev_amp[1] && prev_amp[1] > prev_amp[2] && amplitude < 0.3f) {
            h->params->Kp = Kp;
            h->params->Ki = Ki;
            h->params->Kd = Kd;
            printf("=== AutoTune Converged ===\r\n");
            printf("Kp=%.3f, Ki=%.3f, Kd=%.3f, Tu=%.2fs, Amp=%.2f\r\n",
                   Kp, Ki, Kd, avg_period, amplitude);
            h->relayCtrl(0);
            h->state = TUNE_IDLE;
            return;
        }
    }

    // 超时保护
    if (now - h->start_time > h->duration) {
        h->relayCtrl(0);
        h->state = TUNE_IDLE;
        printf("[AutoTune] Timeout - keeping last PID.\r\n");
    }
}
