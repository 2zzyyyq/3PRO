#ifndef __H_TIM_H__
#define __H_TIM_H__
#ifdef __cplusplus
extern "C" {
#endif
#include "main.h"


#define AC_INPUT_PORT        GPIOA
#define AC_INPUT_PIN         GPIO_PIN_7
#define AC_INPUT_EXTI_LINE   EXTI_LINE1
#define AC_INPUT_PORT_SOURCE GPIOA_PORT_SOURCE
#define AC_INPUT_PIN_SOURCE  GPIO_PIN_SOURCE7
#define AC_INPUT_IRQn        EXTI0_6_IRQn

#define HOLL_INPUT_PORT        GPIOB
#define HOLL_INPUT_PIN         GPIO_PIN_4
#define HOLL_INPUT_EXTI_LINE   EXTI_LINE5
#define HOLL_INPUT_PORT_SOURCE GPIOB_PORT_SOURCE
#define HOLL_INPUT_PIN_SOURCE  GPIO_PIN_SOURCE4
#define HOLL_INPUT_IRQn        EXTI0_6_IRQn


#define TIM_OUTPWM          TIM3
#define TIMx_CLK            RCC_APB_PERIPH_TIM3

#define TIMx_433           TIM1
#define LED1_GPIO_CLK      RCC_APB_PERIPH_GPIO
#define LED1_GPIO_PORT     GPIOF
#define LED1_GPIO_PIN      GPIO_PIN_0
#define LED1_GPIO_AF       GPIO_AF0

#define LED2_GPIO_CLK      RCC_APB_PERIPH_GPIO
#define LED2_GPIO_PORT     GPIOF
#define LED2_GPIO_PIN      GPIO_PIN_1
#define LED2_GPIO_AF       GPIO_AF0
 
#define TIMx_433_GPIO_CLK      RCC_APB_PERIPH_GPIO
#define TIMx_CH3_GPIO_PORT     GPIOB
#define TIMx_CH3_GPIO_PIN      GPIO_PIN_6
#define TIMx_CH3_GPIO_AF       GPIO_AF7

#define TIM_BUZZ                    TIM4

#define TIM4_CH1_GPIO_CLK           RCC_APB_PERIPH_GPIO
#define TIM4_CH1_GPIO_PORT_BUZZ     GPIOA
#define TIMx_CH4_GPIO_PIN_BUZZ      GPIO_PIN_2
#define TIMx_CH4_GPIO_AF_BUZZ       GPIO_AF12
 

#define TIM1_PWM            TIM3
#define LED1_GPIO_CLK       RCC_APB_PERIPH_GPIO
#define MOTOR_GPIO_PORT     GPIOB
#define MOTOR_GPIO_PIN      GPIO_PIN_0
#define MOTOR_GPIO_AF       GPIO_AF11

#define MOTOR2_GPIO_PORT     GPIOB
#define MOTOR2_GPIO_PIN      GPIO_PIN_1
#define MOTOR2_GPIO_AF       GPIO_AF11
 
#define TIMx_IRQn              TIM3_IRQn
#define TIMx_IRQHandler        TIM3_IRQHandler




void TIM3_PWM_LED(void);
void TIM3_PWM_MOTOR(void);
void TIM4_PWM_BUZZ(void);
void TIM1_Input_433(void);
void Set_PWM_Frequency(uint32_t freq_hz);
void TIM_Switch_Capture_Edge( uint32_t edge_type);
void KeyInputExtiInit(GPIO_Module* GPIOx, uint16_t Pin);
void HOLLInputExtiInit(GPIO_Module* GPIOx, uint16_t Pin);
void ADC_StartConversion(void);
void ADC_Initial(void);



#ifdef __cplusplus
}

 

#endif
 




#endif