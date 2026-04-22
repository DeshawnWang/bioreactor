#include "ntc_control.h"

#define SETPOINT 50.0f   // 目标温度，可根据需求修改

static PID_t pid;              // PID控制器
static PID_Params_t tuned_params;  // 自整定得到的参数

void NTC_Control_Init(void) {
    // 运行自整定，持续 5分钟（可以根据需要调整）
    PID_AutoTune(&tuned_params, Read_Temperature, Relay_Switch, SETPOINT, 300000);

    // 用自整定结果初始化 PID
    PID_Init(&pid, tuned_params.Kp, tuned_params.Ki, tuned_params.Kd, -100.0f, 100.0f);

    // 打印最终 PID 参数
    char buf[128];
    snprintf(buf, sizeof(buf), "PID Tuned: Kp=%.2f, Ki=%.2f, Kd=%.2f\r\n",
             tuned_params.Kp, tuned_params.Ki, tuned_params.Kd);
    HAL_UART_Transmit(&huart1, (uint8_t*)buf, strlen(buf), HAL_MAX_DELAY);
}

void NTC_Control_Update(void) {
    float temp = Read_Temperature();
    float control = PID_Task(&pid, SETPOINT, temp, 1.0f); // 假设1秒循环，可改dt

    if (control > 0.0f) {
        Relay_On();
    } else {
        Relay_Off();
    }

    // 串口输出当前状态
    char buf[128];
    snprintf(buf, sizeof(buf), "Temp: %.1fC, Relay: %s, Kp=%.2f Ki=%.2f Kd=%.2f\r\n",
             temp, (control > 0.0f) ? "ON" : "OFF",
             pid.Kp, pid.Ki, pid.Kd);
    HAL_UART_Transmit(&huart1, (uint8_t*)buf, strlen(buf), HAL_MAX_DELAY);
}
