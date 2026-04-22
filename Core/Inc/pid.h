#ifndef PID_H
#define PID_H

#include <stdio.h>
#include <stdint.h>

// ================= PID 基本功能 =================

// PID参数结构体
typedef struct {
    float Kp;
    float Ki;
    float Kd;
} PID_Params_t;

// PID控制器结构体
typedef struct {
    PID_Params_t params;
    float setpoint;
    float last_error;
    float integral;
    float output_min;
    float output_max;
    float output; // 保存最后一次输出，方便调试
    float last_pv;
} PID_HandleTypeDef;

// ================= PID 自整定 =================

// 自整定状态机枚举
typedef enum {
    TUNE_IDLE,       // 空闲
    TUNE_START,      // 开始
    TUNE_HEATING,    // 加热阶段
    TUNE_COOLING,    // 冷却阶段
    TUNE_MEASURE,    // 测周期阶段
    TUNE_CALCULATE   // 计算PID
} AutoTuneState;

// 自整定句柄
typedef struct {
    AutoTuneState state;
    PID_Params_t *params;

    // 回调函数
    float (*readTemp)(void);    // 温度读取函数指针
    void (*relayCtrl)(uint8_t); // 继电器控制函数指针

    // 整定目标
    float target;               // 目标温度
    uint32_t duration;          // 最长自整定时长(ms)

    // 时间信息
    uint32_t start_time;        // 开始时间
    uint32_t last_switch;       // 上次继电器切换时间
    uint32_t last_cross_time;   // 上次过目标点时间
    uint32_t period_accum;      // 周期累计值
    uint32_t period_count;      // 周期计数

    // 温度记录
    float temp_max;             // 最大温度
    float temp_min;             // 最小温度
    float last_temp;            // 上次温度

    // 继电器状态
    uint8_t relay_state;
} AutoTuneHandle;

// ========== 函数声明 ==========

// PID 基本功能
void PID_Init(PID_HandleTypeDef *pid, float Kp, float Ki, float Kd, float min, float max);
float PID_Calculate(PID_HandleTypeDef *pid, float setpoint, float process_var, float dt);
void PID_UpdateParams(PID_HandleTypeDef *pid, float Kp, float Ki, float Kd);

// PID 自整定
void PID_AutoTune_Init(AutoTuneHandle *handle, PID_Params_t *params,
                      float (*readTemp)(void), void (*relayCtrl)(uint8_t),
                      float target, uint32_t duration);
void PID_AutoTune_Task(AutoTuneHandle *handle);
void PID_SelfAdjust(PID_HandleTypeDef *pid, float error, float dTemp);

#endif
