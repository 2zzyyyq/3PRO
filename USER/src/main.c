#include "main.h"
#include "dri_uart2.h"
#include "app_uart2.h"

 //#define OTA
/*不能优化太大*/
/**
*\*\name    main.
*\*\fun     main program.
*\*\param   none
*\*\return  none 
**/
 #ifdef OTA
 
volatile const uint8_t FLASH_DATE[2] __attribute__((section(".ARM.__at_0x08003100")))=
{
		0xA5, 0x5A
};
 #define APP_START_ADDR (0x08003000)

#define MMU_VTOR               ((__IO unsigned*)0x40022050)	//* FLASH->VTOR寄存器地址
#define _VTOREN()              (*MMU_VTOR = (*MMU_VTOR) | 0x80000000)
#define _VTORVALUE()           (*MMU_VTOR = (*MMU_VTOR) | APP_START_ADDR) //*中断向量表重映射地址

#else

#endif
unsigned char debug_buf[80];
int main(void)
  {
			#ifdef OTA
			_VTOREN();

			_VTORVALUE();

			__enable_irq();
			#else
			#endif		
			SystemInit();
			/* System Clocks Configuration */
			RCC_Configuration();
			/* GPIO Configuration */
			GPIO_Configuration();
			/*中断配置*/
			NVIC_Configuration();
		  	/*串口配置*/
			log_init();
			/*电源检测配置*/   
       PVD_Init();
			/*定时器配置*/
    	TIM_Configuration();
	   /*照明定时器配置*/
 			TIM3_PWM_LED();
 		 /*电机定时器配置*/
  		TIM3_PWM_MOTOR();
    	 /*蜂鸣器定时器配置*/
      TIM4_PWM_BUZZ();

		 /*ADC配置*/	
      ADC_Initial();

			read_data();

			variable_init();
			
      r_uart0_protocol_init();
      /*UART2初始化*/
      uart2_init(UART2_BAUDRATE);

      debug_init();  /* 结构化调试日志初始化 (UART2) */

      DEBUG_INFO(MOD_MAIN, "System boot complete, entering main loop");
      
      uart2_protocol_init();
			      /*I/O中断配置*/	
      KeyInputExtiInit(AC_INPUT_PORT, AC_INPUT_PIN);
		 
		 	HOLLInputExtiInit(HOLL_INPUT_PORT, HOLL_INPUT_PIN);
			
		//WDT_Init();

		    // 升级相关处理
    if (Flash_Data.Quite_updata)
    {
      SetupBzhx(LONG_BEEP, 1);
    }
    else // 静默升级后复位
    {
        Net_state = NET_CLOUD;
    }
       Flash_Data.Quite_updata = 1;
    // 设备升级后状态恢复
    if (Flash_Data.Device_updata == 2) // 升级后复位,恢复升级前状态，不上报数据
    {
        // 恢复照明状态，并且不上报  
        Device_State_Data.Light_State_Byte = Flash_Data.Device_light_state;
        Up_State_Data.Light_State_Byte = Device_State_Data.Light_State_Byte;
        
        Device_State_Data.Light_Bright = Flash_Data.Light_pwm_value;
        Up_State_Data.Light_Bright = Device_State_Data.Light_Bright;
        

        Up_State_Data.Motor_State_Byte = Device_State_Data.Motor_State_Byte; // 不上报
			
				Device_State_Data.Motor_State_Byte = MOTOR_SUSPEND_STATE;
			
				Flag.Motor_run_now = 0;

    }
    Flash_Data.Device_updata = 1;
    while (1)
		{
          r_uart0_service();     // wifi处理
          
          uart2_service();     // UART2数据处理

        //  Ovreweight_current_correct();
          limit_protect();       // 限位检测判断
			
          ADC_StartConversion();
			 //掉电保存
			    _220AC_Power_down_save();
 
        if (Flag.time5ms_flag)
        {
					
					  IWDG_ReloadKey();
					
					  Flag.time5ms_flag = 0;
					 
						#ifdef NO_PWM_MOTOR 
					
					 
						#else
					
						Motor_speed();
					
					
						#endif
					
					  capture_433key_judge_answer((button*)&capture_433key, Flag.power_on_10stime_flag); // 检测及响应遥控器
        }
//        
        if (Flag.time10ms_flag)
        {
			 
						#ifdef  TESTADC			
						if(Flag.mcu_wifi_mode)
						{
							volatile uint16_t u32AdcRestult2;
							volatile uint16_t u32AdcRestult3;
							int16_t u32AdcRestult4;
							unsigned char debug_buf[80];
							u32AdcRestult2=Motor_Current.ADvalue*1000/4095*3.6/0.1;	
							snprintf((char*)debug_buf, sizeof(debug_buf), "<any>:%d,%d,%d\r\n",Motor_Current.ADvalue,Ell_Data.Motor_Current_Position,u32AdcRestult2
                                                            );
						  r_uart0_send_bytes(debug_buf, strlen( (const char*)debug_buf ));
						}
						#else  

						#endif
 
          Flag.time10ms_flag = 0;
					
          alarm_signal_test_and_beep();    // 报警信号检测
      
          CmdBzhx(); // 蜂鸣器动作                  

            /* 周期性系统状态输出 (每10ms), 用于软件可靠性分析 */
            {
                static uint8_t cnt_100ms = 0;
                cnt_100ms++;

                /* 电机状态 (每10ms) */
                DEBUG_INFO(MOD_MOTOR, "st=%d pos=%d pulse=%d spd=%d run=%d",
                           Device_State_Data.Motor_State_Byte,
                           Device_State_Data.Motor_Position,
                           Motor_Current_Position_pulse,
                           Device_State_Data.current_speed,
                           Flag.Motor_run_now);
                /* 电流与故障 (每10ms) */
                DEBUG_INFO(MOD_ADC, "adc=%d err=0x%02X(obs=%d ow=%d stall=%d)",
                           Motor_Current.ADvalue,
                           Err.data,
                           Err.err_bit.Encounter_Obstacle,
                           Err.err_bit.Over_Wight,
                           Err.err_bit.Motor_Err);
                /* 电机运行时打印实际电流值 */
                if (Flag.Motor_run_now)
                {
                    uint16_t cur_x100 = (uint32_t)Motor_Current.ADvalue * 3600 / 4095;
                    DEBUG_INFO(MOD_ADC, "I_motor=%d.%02dA (adc=%d)",
                               cur_x100 / 100, cur_x100 % 100,
                               Motor_Current.ADvalue);
                }
                /* 限位 (每10ms) */
                DEBUG_INFO(MOD_LIMIT, "up_lim=%d dn_lim=%d yz=%d",
                           Flag.Up_limit,
                           Flag.Down_limit,
                           Flag.Yz);
                /* WiFi/网络状态 (每100ms) */
                if (cnt_100ms >= 10)
                {
                    cnt_100ms = 0;
                    DEBUG_INFO(MOD_WIFI, "net=%d step=%d tick=%lu",
                               Net_state,
                               g_DevStatus.Poweron_Set_Model_step,
                               (unsigned long)g_debug_tick_ms);
                }
            }

        }
        if (Flag.time100ms_flag)
        {
			     
            Flag.time100ms_flag = 0;
 
            PowerOn_ModelSet();
        }
        
        /* 每个150ms循环执行 */
        if (Flag.time150ms_flag)
        {
           Flag.time150ms_flag = 0;
					
		      _150ms_work();                   // wifi状态回馈
        }
		};
}

/**
*\*\name    RCC_Configuration.
*\*\fun     Configures the different system clocks.
*\*\param   none
*\*\return  none 
**/
void RCC_Configuration(void)
{
		//RCC_DeInit();
		/* TIM_OUTPWM, GPIOx clocks enable */
		RCC_EnableAPBPeriphClk(TIMx_CLK|RCC_APB_PERIPH_TIM1|RCC_APB_PERIPH_TIM4,ENABLE);
		RCC_EnableAPBPeriphClk(LED1_GPIO_CLK | LED2_GPIO_CLK |\
	                          	TIMx_433_GPIO_CLK | TIM4_CH1_GPIO_CLK,ENABLE);
		RCC_EnableAPBPeriphClk(RCC_APB_PERIPH_TIM6,ENABLE);
		RCC_EnableAPBPeriphClk(RCC_APB_PERIPH_GPIO,ENABLE);
	
		/* Enable ADC clocks */
		RCC_EnableAHBPeriphClk(RCC_AHB_PERIPH_ADC, ENABLE);

		/* RCC_ADCHCLK_DIV4*/
		ADC_ClockModeConfig(RCC_ADCHCLK_DIV4);
}



/**
*\*\name    NVIC_Configuration.
*\*\fun     Configures the nested vectored interrupt controller.
*\*\param   none
*\*\return  none 
**/
void NVIC_Configuration(void)
{
    NVIC_InitType NVIC_InitStructure;

 
    EXTI_InitType EXTI_Struct;

    EXTI_Struct.EXTI_Line    = EXTI_LINE9;
    EXTI_Struct.EXTI_Mode    = EXTI_Mode_Interrupt;
    EXTI_Struct.EXTI_Trigger = EXTI_Trigger_Rising_Falling;
    EXTI_Struct.EXTI_LineCmd = ENABLE;
    EXTI_InitPeripheral(&EXTI_Struct);
    
    /* Enable the TIM_OUTPWM Interrupt */
    NVIC_InitStructure.NVIC_IRQChannel                   = TIM6_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPriority           = 3;
    NVIC_InitStructure.NVIC_IRQChannelCmd                = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
 
    /* Enable the TIM_OUTPWM Interrupt */
    NVIC_InitStructure.NVIC_IRQChannel                    = TIM1_CC_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPriority            = 3;
    NVIC_InitStructure.NVIC_IRQChannelCmd                 = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    NVIC_InitStructure.NVIC_IRQChannel                   = ADC_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPriority           = 2;
    NVIC_InitStructure.NVIC_IRQChannelCmd                = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

		    /* Enable the UARTy Interrupt */
    NVIC_InitStructure.NVIC_IRQChannel                   = UART1_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPriority           = 1;
    NVIC_InitStructure.NVIC_IRQChannelCmd                = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
		
				    /* Enable the UARTy Interrupt */
    NVIC_InitStructure.NVIC_IRQChannel                   = UART2_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPriority           = 1;
    NVIC_InitStructure.NVIC_IRQChannelCmd                = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

}





 

















