
/**
*\*\file N32G033_it.c
*\*\author Nsing
*\*\version v1.0.0
*\*\copyright Copyright (c) 2025, Nsing Technologies Inc. All rights reserved.
**/
#include "n32g033_it.h"
#include "main.h"
#include "dri_uart2.h"
#include <stdlib.h>
/*** Cortex-M4 Processor Exceptions Handlers ***/
 
 
/**
*\*\name    NMI_Handler.
*\*\fun     This function handles NMI exception.
*\*\param   none
*\*\return  none 
**/
void NMI_Handler(void)
{
   
}

/**
*\*\name    HardFault_Handler.
*\*\fun     This function handles Hard Fault exception.
*\*\param   none
*\*\return  none 
**/
void HardFault_Handler(void)
{
    /* Go to infinite loop when Hard Fault exception occurs */
    DEBUG_ERROR(MOD_MAIN, "!!! HARDFAULT !!! system halted");
    while (1)
    {
    }
}

/**
*\*\name    MemManage_Handler.
*\*\fun     This function handles Memory Manage exception.
*\*\param   none
*\*\return  none 
**/
void MemManage_Handler(void)
{
    /* Go to infinite loop when Memory Manage exception occurs */
    while (1)
    {
    }
}

/**
*\*\name    BusFault_Handler.
*\*\fun     This function handles Bus Fault exception.
*\*\param   none
*\*\return  none 
**/
void BusFault_Handler(void)
{
    /* Go to infinite loop when Bus Fault exception occurs */
    while (1)
    {
    }
}

/**
*\*\name    UsageFault_Handler.
*\*\fun     This function handles Usage Fault exception.
*\*\param   none
*\*\return  none 
**/
void UsageFault_Handler(void)
{
    /* Go to infinite loop when Usage Fault exception occurs */
    while (1)
    {
    }
}

/**
*\*\name    SVC_Handler.
*\*\fun     This function handles SVCall exception.
*\*\param   none
*\*\return  none 
**/
void SVC_Handler(void)
{
}

/**
*\*\name    DebugMon_Handler.
*\*\fun     This function handles Debug Monitor exception.
*\*\param   none
*\*\return  none 
**/
void DebugMon_Handler(void)
{
}

/**
*\*\name    SysTick_Handler.
*\*\fun     This function handles SysTick Handler.
*\*\param   none
*\*\return  none 
**/
void SysTick_Handler(void)
{   

}

void TIM1_CC_IRQHandler(void)
{
   HAL_TIM1_CC_IRQHandler_BACK();
}

 


void ADC_IRQHandler(void)
{
   HAL_ADC_IRQHandler_BACK();
}
/**
*\*\name    TIM6_IRQHandler.
*\*\fun     This function handles TIM_OUTPWM interrupts.
*\*\param   none
*\*\return  none 
**/
void TIM6_IRQHandler(void)
{
	
	  if (EXTI_GetITStatus(EXTI_LINE9) != RESET)
    {
 
			 EXTI_ClrITPendBit(EXTI_LINE9);
 
    }
    if (TIM_GetIntStatus(TIM6, TIM_INT_UPDATE) != RESET)
    {
				TIM_ClrIntPendingBit(TIM6, TIM_INT_UPDATE);
			 	EXTI_ClrITPendBit(EXTI_LINE9);
        HAL_TIM_CNT_IRQHandler_BACK();
    }
		

}

/* n32g033 Peripherals Interrupt Handlers, interrupt handler's name please refer to the startup file (startup_n32g033.s). */

/**
*\*\name    EXTI0_6_IRQHandler.
*\*\fun     External lines 0 to 6 interrupt.
*\*\param   none
*\*\return  none 
**/
 extern u8 Mode_PWM ;
void EXTI0_6_IRQHandler(void)
{
     HAL_EXIT_IRQHandler_BACK();
}
volatile uint8_t * gp_uart0_tx_address;         /* uart0 send buffer address */
volatile uint16_t  g_uart0_tx_count;            /* uart0 send data number */
volatile uint8_t * gp_uart0_rx_address;         /* uart0 receive buffer address */
volatile uint16_t  g_uart0_rx_count;            /* uart0 receive data number */
volatile uint16_t  g_uart0_rx_length;           /* uart0 receive data length */
uint8_t send_busy=0;
 // 全局变量（放在文件头部）
extern unsigned char *uart_tx_ptr ;   // 保存src_p
extern unsigned char uart_tx_len  ;      // 保存dataNum
#ifdef  DUG_UART
void UART2_IRQHandler(void)
{
    __IO uint8_t rx_data;
    if((UART_GetFlagStatus(LOG_UARTx, UART_FLAG_RXDNE)!= RESET))         //UART1数据接收
    {

            UART_ClrFlag(LOG_UARTx, UART_FLAG_RXDNE);        //清中断状态位
            rx_data = UART_ReceiveData(LOG_UARTx);   //接收数据字节
            r_uart0_receive_input(rx_data);

    }

    if(UART_GetFlagStatus(LOG_UARTx, UART_FLAG_TXC))         //UART1数据发送
    {

        UART_ClrFlag(LOG_UARTx, UART_FLAG_TXC);        //清中断状态位

            if (g_uart0_tx_count > 0U)
                {

                    UART_SendData(LOG_UARTx,*gp_uart0_tx_address);         //调用库函数，通过UART0发送一个字母。
                    gp_uart0_tx_address++;
                    g_uart0_tx_count--;
                }
                else
                {
                  send_busy=0;
                }

    }

}
#else
void UART1_IRQHandler(void)
{
    uint8_t rx_data;
    if((UART_GetFlagStatus(LOG_UARTx, UART_FLAG_RXDNE)!= RESET))         //UART1数据接收
    {

            UART_ClrFlag(LOG_UARTx, UART_FLAG_RXDNE);        //清中断状态位
            rx_data = UART_ReceiveData(LOG_UARTx);   //接收数据字节
            r_uart0_receive_input(rx_data);

    }

    if(UART_GetFlagStatus(LOG_UARTx, UART_FLAG_TXC))         //UART1数据发送
    {

        UART_ClrFlag(LOG_UARTx, UART_FLAG_TXC);        //清中断状态位

        if (g_uart0_tx_count > 0U)
        {

            UART_SendData(LOG_UARTx,*gp_uart0_tx_address);         //调用库函数，通过UART0发送一个字母。
            gp_uart0_tx_address++;
            g_uart0_tx_count--;
        }
        else
        {
            send_busy=0;
        }

    }

}

/**
 * @brief  UART2 中断服务函数（独立于 UART1，使用 UART2 硬件直接访问）
 * @note   接收字节推入 UART2 独立环形队列；发送完成处理 UART2 TX 状态
 */
void UART2_IRQHandler(void)
{
    if (UART_GetFlagStatus(UART2, UART_FLAG_RXDNE) != RESET)
    {
        UART_ClrFlag(UART2, UART_FLAG_RXDNE);
        uart2_ringbuf_put(UART_ReceiveData(UART2));
    }

    if (UART_GetFlagStatus(UART2, UART_FLAG_TXC) != RESET)
    {
        UART_ClrFlag(UART2, UART_FLAG_TXC);

        if (uart2_tx_count > 0U)
        {
            UART_SendData(UART2, *uart2_tx_ptr);
            uart2_tx_ptr++;
            uart2_tx_count--;
        }
        else
        {
            uart2_tx_busy = 0;
        }
    }
}
#endif
void PVD_IRQHandler(void)
{
 if (EXTI_GetITStatus(EXTI_LINE7) != RESET)
    {
			EXTI_ClrITPendBit(EXTI_LINE7);
		    DEBUG_ERROR(MOD_PVD, "Power down detected! Saving data to flash...");
			MOTOR_DN_IO(0);//停止电机
			MOTOR_UP_IO(0);
			B_MOTOR_DN_IO(0);
			B_MOTOR_UP_IO(0);
			BuzVcc_OFF;
			BUZ_OFF;
			LED_LIGHT_OFF;

    //	Flag.Low_power=1;
			FDL_Block0_Save_Times=FDL_ONCE_MAX_WRITE_NUM-1;	
			Flash_WriteStruct(FDL_Block0+FDL_Block0_Save_Times*sizeof(Flash_Data),(uint8_t*)&Flash_Data,sizeof(Flash_Data));//写入一次	
			FDL_Block1_Save_Times=FDL_BLOCK1_ONCE_MAX_WRITE_NUM-1;
			Flash_WriteStruct(FDL_Block1+FDL_Block1_Save_Times*sizeof(Ell_Data),(uint8_t*)&Ell_Data,sizeof(Ell_Data));//块1写入一次	

			while(1)
			{
			
			
			};
    }
 

}


// 全局位置变量
/*
volatile int32_t g_encoder_position = 0;     // 32位扩展位置
volatile uint16_t g_last_counter = 0;        // 上一次计数器值
volatile int8_t g_encoder_direction = 0;     // 方向：1=正，-1=负，0=未知
volatile uint32_t g_overflow_count = 0;      // 溢出次数
// TIM4中断服务程序
void TIM4_IRQHandler(void)
{
    static uint16_t last_cnt = 0;
    
    // 检查更新中断（溢出中断）
    if (TIM_GetIntStatus(TIM4, TIM_INT_UPDATE) == SET)
    {
        // 获取当前计数值
        uint16_t current_cnt = TIM_GetCnt(TIM4);
        
        // 判断溢出方向
        if (current_cnt < last_cnt) 
        {
            // 正向溢出（从接近ARR值回绕到0）
            g_encoder_direction = 1;
            g_overflow_count++;
        } 
        else 
        {
            // 负向溢出（从0回绕到ARR值）
            g_encoder_direction = -1;
            g_overflow_count--;
        }
        
        // 清除中断标志
        TIM_ClrIntPendingBit(TIM4, TIM_INT_UPDATE);
        TIM_ClearFlag(TIM4, TIM_FLAG_UPDATE);
        
        last_cnt = current_cnt;
    }
}


*/















