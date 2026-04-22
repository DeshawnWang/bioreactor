#include "ntc_control.h"
#include "pid.h"
#include "relay.h"
#include "temperature.h"
#include "usart.h"
#include <math.h>
#include <string.h>
#include "pt100.h"

// ==================== 参数定义 ====================
#define SETPOINT                30.0f
#define PRE_ADJUST_TOLERANCE    0.5f      // 减小预热容差
#define PREHEAT_TIMEOUT         60000
#define AUTO_TUNE_DURATION      300000
#define FORCE_HEATING_THRESHOLD (SETPOINT - 0.3f)  // 动态阈值

// 添加预测控制参数
#define TEMP_RISE_RATE_THRESHOLD 0.02f    // ℃/s 温度上升率阈值
#define EARLY_STOP_MARGIN       0.05f     // 提前停止加热的余量

// ==================== 全局变量 ====================
PID_HandleTypeDef pid;
PID_Params_t pid_params = { .Kp = 1.5f, .Ki = 0.1f, .Kd = 0.8f };  // 更保守的初始参数
AutoTuneHandle tune_handle;

TempStage_t temp_stage = TEMP_STAGE_PREHEAT;
static uint32_t preheat_start_time = 0;

// 添加预测控制变量
static float last_temp = 0.0f;
static uint32_t last_temp_time = 0;
static float temp_rise_rate = 0.0f;

// ==================== 初始化 ====================
void NTC_Control_Init(void)
{
    printf("\r\n=== [NTC] Temperature Control Init ===\r\n");
    printf("Target: %.2f°C | Precision: ±0.1°C\r\n", SETPOINT);

    temp_stage = TEMP_STAGE_PREHEAT;
    preheat_start_time = HAL_GetTick();
    last_temp = 0.0f;
    last_temp_time = HAL_GetTick();
    temp_rise_rate = 0.0f;

    Relay_Switch(1);
    printf("[NTC] Enter PREHEAT stage...\r\n");
}

// ==================== 主循环任务 ====================
void NTC_Control_Update(void)
{
    float temp = PT100_Task();
    uint32_t now = HAL_GetTick();

    // 计算温度变化率（用于预测控制）
    if(last_temp_time > 0 && now > last_temp_time) {
        float dt = (now - last_temp_time) / 1000.0f;  // 转换为秒
        if(dt > 0.1f && dt < 10.0f) {  // 避免除零和过大时间间隔
            temp_rise_rate = (temp - last_temp) / dt;
        }
    }
    last_temp = temp;
    last_temp_time = now;

    // ---------- 阶段1：预热 ----------
    if (temp_stage == TEMP_STAGE_PREHEAT) {
        float diff = SETPOINT - temp;

        if (diff <= PRE_ADJUST_TOLERANCE && diff >= 0) {
            Relay_Switch(0);
            printf("[NTC] Preheat done (%.3f°C), start PID AutoTune...\r\n", temp);

            // 使用更保守的初始PID参数
            PID_Init(&pid, 1.5f, 0.2f, 0.5f, -100, 100);
            PID_AutoTune_Init(&tune_handle, &pid_params,
                              PT100_Task, Relay_Switch,
                              SETPOINT, AUTO_TUNE_DURATION);

            temp_stage = TEMP_STAGE_AUTOTUNE;
            return;
        }

        if (now - preheat_start_time > PREHEAT_TIMEOUT) {
            Relay_Switch(0);
            printf("[NTC] Preheat timeout after %.1fs, using conservative PID.\r\n",
                   PREHEAT_TIMEOUT / 1000.0f);

            // 使用保守的PID参数
            PID_Init(&pid, 1.5f, 0.1f, 0.8f, -100, 100);
            temp_stage = TEMP_STAGE_PID;
            return;
        }

        Relay_Switch(temp < SETPOINT ? 1 : 0);
        printf("[NTC] Preheating... %.3f°C (Target: %.1f°C)\r\n", temp, SETPOINT);
        return;
    }

    // ---------- 阶段2：自整定 ----------
    if (temp_stage == TEMP_STAGE_AUTOTUNE) {
        PID_AutoTune_Task(&tune_handle);

        if (tune_handle.state == TUNE_IDLE) {
            printf("[NTC] PID AutoTune complete.\r\n");
            printf("Tuned Params: Kp=%.3f Ki=%.3f Kd=%.3f\r\n",
                   pid_params.Kp, pid_params.Ki, pid_params.Kd);

            PID_UpdateParams(&pid, pid_params.Kp, pid_params.Ki, pid_params.Kd);
            temp_stage = TEMP_STAGE_PID;
        }
        return;
    }

    // ---------- 阶段3：精确PID控制 ----------
    if (temp_stage == TEMP_STAGE_PID) {
        float output = PID_Calculate(&pid, SETPOINT, temp, 1.0f);

        // 智能继电器控制逻辑
        uint8_t should_heat = 0;

        if (temp < SETPOINT - 0.08f) {
            // 温度明显低于设定值，强制加热
            should_heat = 1;
        }
        else if (output > 10.0f && temp < SETPOINT - 0.03f) {
            // PID输出较大且温度略低于设定值
            should_heat = 1;
        }
        else if (temp_rise_rate > TEMP_RISE_RATE_THRESHOLD &&
                 temp > SETPOINT - EARLY_STOP_MARGIN) {
            // 温度上升较快且接近设定值，提前停止加热
            should_heat = 0;
        }
        else if (output > 0 && temp < SETPOINT + 0.05f) {
            // 温和加热，避免超调
            should_heat = 1;
        }

        Relay_Switch(should_heat);

        // 调试信息（减少打印频率）
        static uint32_t last_debug_time = 0;
        if (now - last_debug_time >= 2000) {
            printf("[PID] SP=%.1f°C | PV=%.3f°C | Out=%.1f | Rate=%.3f°C/s | Heat=%d\r\n",
            		SETPOINT,temp, output, temp_rise_rate, should_heat);
            last_debug_time = now;
        }
    }
}
