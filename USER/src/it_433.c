#include "main.h"
#include <stdlib.h>  
void HAL_TIM1_CC_IRQHandler_BACK(void);
volatile TIME_SLICE  Time;
__IO uint16_t PWM_cycle  = 0;
__IO uint16_t DutyCycle = 0;
__IO uint32_t Frequency = 0;
__IO uint16_t Hlevel  = 0;

#ifdef D_433
uint32_t LowLevelTime=0;
void HAL_TIM1_CC_IRQHandler_BACK(void)
{
	    TIM_ClrIntPendingBit(TIMx_433, TIM_INT_CC2);
			/* Clear TIM_OUTPWM Capture compare interrupt pending bit */
			/* Get the Input Capture value */
			PWM_cycle = TIM_GetCap2(TIMx_433);
	        Hlevel=TIM_GetCap1(TIMx_433) ;
	    if(Hlevel!=0)
			{
				Record_PulseWidth(Hlevel,&Capture_Command);
			}
			if (PWM_cycle != 0)
			{
				PWM_cycle = TIM_GetCap2(TIMx_433);
				Hlevel=TIM_GetCap1(TIMx_433) ;
				LowLevelTime = PWM_cycle - Hlevel;	
				Record_PulseWidth(LowLevelTime,&Capture_Command);
			}
			else
			{
				DutyCycle = 0;
				Frequency = 0;
			}
}
#else

/**
*\*\name    TIMx_IRQHandler.
*\*\fun     This function handles TIM_OUTPWM interrupts.
*\*\param   none
*\*\return  none 
**/
/* 全局变量存储结果 */
volatile struct {
    uint32_t high_us;      // 高电平时间（us）
    uint32_t low_us;       // 低电平时间（us）
    uint32_t period_us;    // 周期（us）
    uint32_t frequency;    // 频率（Hz）
    uint8_t duty_percent;  // 占空比（%）
    uint8_t updated;       // 数据更新标志
} pwm_result = {0};
uint16_t date[256]={0};
/* 完整测量并存储到全局变量 */
void TIM1_CC_IRQHandler(void)
{   
    volatile static uint8_t step = 0;           // 0:等待R, 1:等待F, 2:等待第二个R
    volatile static uint32_t r1 = 0, f = 0, r2 = 0;
    volatile static uint32_t overflow = 0;
    volatile static uint32_t high_ticks = 0;
    volatile static uint32_t i = 0;
    /* 处理溢出中断 */
    if (TIM_GetIntStatus(TIMx_433, TIM_INT_UPDATE) != RESET)
    {
        TIM_ClrIntPendingBit(TIMx_433, TIM_INT_UPDATE);
        overflow++;
    }
    
    /* 处理捕获中断 */
    if (TIM_GetIntStatus(TIMx_433, TIM_INT_CC2) != RESET)
    {
        TIM_ClrIntPendingBit(TIMx_433, TIM_INT_CC2);
        
        volatile uint32_t now = TIM_GetCap2(TIMx_433);
        volatile uint32_t diff;
        
        switch(step)
        {
            case 0:  // 第一个上升沿
                r1 = now;
                overflow = 0;
                TIM_Switch_Capture_Edge(TIM_IC_POLARITY_FALLING);
                step = 1;
									if(i>255)
									{
									  i=0;
									}
                break;
                
            case 1:  // 下降沿
                f = now;
                
                // 计算高电平时间
                if (f >= r1)
                {
                    diff = f - r1;
                }
                else
                {
                    diff = (0xFFFF - r1) + f + 1;
                    diff += overflow * 0x10000;
                }
                i++;
                high_ticks = diff;
                pwm_result.high_us = diff;  // 1us/计数
                date[i]=pwm_result.high_us;
                TIM_Switch_Capture_Edge(TIM_IC_POLARITY_RISING);
                step = 2;
                break;
                
            case 2:  // 第二个上升沿
                r2 = now;
                
                // 计算低电平时间
                if (r2 >= f)
                {
                    diff = r2 - f;
                }
                else
                {
                    diff = (0xFFFF - f) + r2 + 1;
                    diff += overflow * 0x10000;
                }
                i++;
                pwm_result.low_us = diff;
                date[i]= pwm_result.low_us;
                // 计算完整周期
                pwm_result.period_us = pwm_result.high_us + diff;
                
                // 计算频率和占空比
                if (pwm_result.period_us > 0)
                {
                    pwm_result.frequency = 1000000 / pwm_result.period_us;
                    pwm_result.duty_percent = (pwm_result.high_us * 100) / pwm_result.period_us;
                    pwm_result.updated = 1;  // 标记数据已更新
                }
                
                // 重置
                step = 0;
                overflow = 0;
                break;
        }
    }
}



#endif
 



















































































