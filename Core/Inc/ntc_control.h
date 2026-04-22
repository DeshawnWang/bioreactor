#ifndef NTC_CONTROL_H
#define NTC_CONTROL_H

#include "pid.h"

// ====================== 温控阶段定义 ======================
typedef enum {
    TEMP_STAGE_PREHEAT = 0,   // 预热阶段
    TEMP_STAGE_AUTOTUNE,      // PID自整定阶段
    TEMP_STAGE_PID            // 正常PID控制阶段
} TempStage_t;

// ====================== 外部变量声明 ======================
extern TempStage_t temp_stage;
extern PID_HandleTypeDef pid;
extern PID_Params_t pid_params;

// ====================== 函数声明 ======================
void NTC_Control_Init(void);
void NTC_Control_Update(void);

#endif
