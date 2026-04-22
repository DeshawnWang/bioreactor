//#include "OD.h"
//#include "ads8688.h"
//
//// ---------- LED 引脚 ----------
//#define LED_PORT GPIOB
//#define LED_PIN  GPIO_PIN_8
//
//int32_t data1 = 0;
//int32_t data2 = 0;
//
////=======OD¼ÆËã======//
//#define Min_Interation 20
//#define Max_Interation 40
//int16_t interation_count = Max_Interation;
//uint32_t total_value = 0;
//float baseline_value = 0;
//float od_result;
//float min_log_value = 0;
//int32_t raw_result = 0;
//
//float Q = 0.01;  //
//float R = 1.0;   //
//float x = 0.0;   //
//float P = 1.0;   //
//float K = 0.0;   //
//
//
////=====¿ª¹Ø=====//
//int8_t switch_state= 0;
//
//// ---------- 内部函数 ----------
//static float KalmanFilter(float measurement)
//{
//    P = P + Q;
//    K = P / (P + R);
//    x = x + K * (measurement - x);
//    P = (1 - K) * P;
//    return x;
//}
//
//// ---------- 初始化 ----------
//void OD_Init(void)
//{
//    interation_count = 0;
//    total_value = 0;
//    baseline_value = 0;
//    min_log_value = 0;
//    od_result = 0;
//    x = 0;
//    P = 1.0f;
//    switch_state = 1;
//}
//
//int32_t Get_Data()
//{
//    int32_t results1 = 0;
//    int32_t results2 = 0;
//    int32_t results = 0;
//    // ¿ªLED
//    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6, GPIO_PIN_SET);
//    HAL_Delay(1500);
//
//    for (int i = 0; i < 40; i++)
//    {   uint16_t adc_data = 0;
//    	Get_MAN_CH_Data(MAN_CH_2, &adc_data);  // CH2 ±39.0625mV
//
//         data1 = adc_data;
//        if(data1 == INT32_MIN)
//        {
//            return 0;
//        }
//        results1 = results1 + data1;
//       printf("result1:%ld\n",results1);
//    }
//
//    // ¹ØLED
//    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6, GPIO_PIN_RESET);
//    HAL_Delay(1500);
//    for (int i = 0; i < 40; i++)
//    {
//    	uint16_t adc_data = 0;
//    	Get_MAN_CH_Data(MAN_CH_2, &adc_data);  // CH2 ±39.0625mV
//        data2 = adc_data ;
//        if(data2 == INT32_MIN)
//        {
//            return 0;
//        }
//        results2 = results2 + data2;
//      printf("result2:%ld\n",results2);
//    }
//
//    results = ((results1 - results2) / 40);
//    return results;
//}
//
//void OD_Task(void){
//	if(switch_state)
//					{
//		//printf("OD_Task executed, switch_state: %d\n", switch_state); // 新增调试打印
//	        raw_result = Get_Data();
//
//	        if (interation_count < Max_Interation - Min_Interation)
//	        {
//	            interation_count++;
//
//	        }
//	        else if (interation_count >= Max_Interation - Min_Interation && interation_count < Max_Interation)
//	        {
//	            interation_count++;
//	            total_value += raw_result;
//	            baseline_value = (float)total_value / (interation_count - Min_Interation);
//	            printf("OD Baseline Calculated,baseline_value: %.6f \r\n", baseline_value);
//	        }
//	        else
//	        {
//
//	            if (raw_result <= 0)
//	            {
//	                od_result = od_result;
//	                printf("OD_value: %.6f ,baseline_value: %.6f \r\n",od_result, baseline_value);
//	            }
//	            else if (raw_result < baseline_value)
//	            {
//	//								float log_value = log10(raw_result / baseline_value);
//
//	//								// ¼ÇÂ¼×îÐ¡logÖµ£¬ÓÃÓÚÖ®ºóÕûÌåÌ§Éý
//	//								if (log_value < min_log_value) {
//	//										min_log_value = log_value;
//	//								}
//	//								// Ì§ÉýºóµÄODÖµ£¨È·±£×îÐ¡ÖµÎª0£©
//	//								od_result = 14 * (log_value - min_log_value);
//									float log_value = log10( baseline_value/ raw_result);
//									if (log_value > min_log_value)
//										{
//											min_log_value = log_value;
//										}
//	                od_result = log_value;
//	                printf("OD_value: %.6f ,baseline_value: %.6f \r\n",od_result, baseline_value);
//	            }
//							else
//							{
//								od_result = min_log_value + log10(raw_result / baseline_value);
//								printf("OD_value: %.6f ,baseline_value: %.6f \r\n",od_result, baseline_value);
//							}
//	            float filtered_value = KalmanFilter(od_result);
//							if (filtered_value < 0) filtered_value = 0;
//							printf("filtered_OD_value: %.6f ,baseline_value: %.6f \r\n",filtered_value, baseline_value);
//	        }
//				}
//}
////// ---------- OD 更新 ----------
////void OD_Update(void)
////{
////    uint16_t adc_data;
////
////    // 1. 开灯测量
////    HAL_GPIO_WritePin(LED_PORT, LED_PIN, GPIO_PIN_SET);
////    HAL_Delay(1500); // 稳定时间
////    Get_MAN_CH_Data(MAN_CH_2, &adc_data);  // CH2 ±39.0625mV
////
////    float voltage_on = ((float)adc_data / 32768.0f - 1.0f) * 2.5f;
////
////    // 2. 关灯测量
////    HAL_GPIO_WritePin(LED_PORT, LED_PIN, GPIO_PIN_RESET);
////    HAL_Delay(1500);
////    Get_MAN_CH_Data(MAN_CH_2, &adc_data);
////    float voltage_off = ((float)adc_data / 32768.0f - 1.0f) * 2.5f;
////
////    float raw_result = voltage_on - voltage_off;
////    ch2_voltage = voltage_on; // 保存灯亮电压值
////
////    // ---------- baseline 计算 ----------
////    if (interation_count < MAX_ITERATION)
////    {
////        total_value += (uint32_t)(raw_result * 100000.0f); // 放大为整数计算
////        iteration_count++;
////        if (interation_count >= MIN_ITERATION)
////        {
////            baseline_value = (float)total_value / iteration_count / 100000.0f;
////            printf("OD Baseline Calculated: %.6f V (iteration %d)\r\n", baseline_value, iteration_count);
////        }
////        else
////        {
////            printf("OD Baseline Collecting: %.6f V (iteration %d)\r\n", (float)total_value / iteration_count / 100000.0f, iteration_count);
////        }
////        return;
////    }
////
////
////    // ---------- OD计算 ----------
////    if (raw_result <= 0) return;
////
////    if (raw_result < baseline_value)
////    {
////        float log_value = log10f(baseline_value / raw_result);
////        if (log_value > min_log_value) min_log_value = log_value;
////        od_result = log_value;
////    }
////    else
////    {
////        od_result = min_log_value + log10f(raw_result / baseline_value);
////    }
////
////    od_result = KalmanFilter(od_result);
////    if (od_result < 0) od_result = 0;
////
////    printf("OD=%.3f, Voltage=%.5f V, Baseline=%.6f V\r\n", od_result, ch2_voltage, baseline_value);
////}
//
////// ---------- 获取当前 OD ----------
////float OD_GetValue(void)
////{
////    return od_result;
////}
////
////// ---------- 获取当前电压 ----------
////float OD_GetVoltage(void)
////{
////    return ch2_voltage;
////}
