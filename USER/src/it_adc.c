#include "main.h"
#include "ALL.h" 
// 全局变量保存ADC值
volatile uint16_t adc_value_pb2 = 0;  // PB2的ADC值
volatile uint16_t adc_value_pa0 = 0;  // PA0的ADC值
volatile uint8_t adc_conversion_complete = 0;
volatile FUNCTION_SELECT  Function_select,Motor_Current,Motor_Current_B;
 OVERWEIGHT_CORRECT Overweight_Correct;

void HAL_ADC_IRQHandler_BACK(void);
void HAL_ADC_IRQHandler_BACK(void)
{
	      /* 检查转换完成中断 */
    if (ADC_GetIntStatus(ADC_INT_FLAG_ENDC) != RESET)
    {
					/* 读取两个通道的ADC值 */
					/* 注意：GetGroupConvDat参数是0-9，对应Rank 1-10 */
					// adc_value_pb2 = ADC_GetGroupConvDat(0);  // Rank 1: PB2 (通道10)
					// adc_value_pa0 = ADC_GetGroupConvDat(1);  // Rank 2: PA0 (通道0)
					adc_value_pa0 = ADC_GetGroupConvDat(0);  // Rank 2: PA0 (通道0)
					/* 设置转换完成标志 */
					adc_conversion_complete = 1;
					Motor_Current.AD_sum+=(uint16_t)(ADC_GetGroupConvDat(0)); 

					Flag.AD19_Change_start=0;
					Motor_Current.AD_sumple_num++;
				 if(Motor_Current.AD_sumple_num>=100)
				 {
					Motor_Current.AD_sumple_num=0;
					Motor_Current.ADvalue=Motor_Current.AD_sum/100;	
					Motor_Current.AD_sum=0;	
					if((Time.Overweight_Current_time>10)&&(Flag.Overweight_current_correct_start))//校正启动1s后
					{
						Overweight_Correct.Sample_num++;
						Overweight_Correct.Sample_sum+=Motor_Current.ADvalue;
					}
				 }
         /* 清除中断标志 */
         ADC_ClearIntPendingBit(ADC_INT_FLAG_ENDC);
        
        /* 如果需要连续转换，可以在这里重新启动转换 */
         ADC_EnableSoftwareTrig(ENABLE);
    }
//     if(ADC_GetIntStatus(ADC_INT_FLAG_PHS2) != RESET)
//		 {
//		 
//		 
//				ADC_ClearIntPendingBit(ADC_INT_FLAG_PHS2);

//				/* 注意：GetGroupConvDat参数是0-9，对应Rank 1-10 */
//			//	adc_value_pb2 = ADC_GetGroupConvDat(0);  // Rank 1: PB2 (通道10)
//		
//				/* 如果需要连续转换，可以在这里重新启动转换 */
//				ADC_EnableSoftwareTrig(ENABLE);
//        ADC_EnableChannelStartConv(ADC_SWSTRRCH_PHS2_START);
//		 
//		 }
//		 
//    if(ADC_GetIntStatus(ADC_INT_FLAG_PHS1) != RESET)
//		 {
// 
//		    ADC_ClearIntPendingBit(ADC_INT_FLAG_PHS1);

//				/* 注意：GetGroupConvDat参数是0-9，对应Rank 1-10 */
//				adc_value_pb2 = ADC_GetGroupConvDat(0);  // Rank 1: PB2 (通道10)
//				//adc_value_pa0 = ADC_GetGroupConvDat(1);  // Rank 2: PA0 (通道0)
//				/* 如果需要连续转换，可以在这里重新启动转换 */
//				ADC_EnableSoftwareTrig(ENABLE);
//        ADC_EnableChannelStartConv(ADC_SWSTRRCH_PHS1_START);
//		 
//		 }
    /* 检查其他中断标志（可选） */
    if (ADC_GetIntStatus(ADC_INT_FLAG_AWDG) != RESET)
    {
        /* 处理模拟看门狗中断 */
        ADC_ClearIntPendingBit(ADC_INT_FLAG_AWDG);
    }
}


































