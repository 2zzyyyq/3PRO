#ifndef __H_433_H__
#define __H_433_H__
#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"
 
  //#define  TESTADC
//#define   NO_PWM_MOTOR
  //#define  DUG_UART

//#define  XIMI               
//#define   LIMIT_OVER_VALVE

 #define   FALSE 0
 #define   TRUE 1

/********************************GPIO引脚定义********************************/
#define YZ_PIN               GPIO_PIN_6
#define YZ_PORT              GPIOA

#ifdef  DUG_UART
#define LOG_UARTx       UART2
#define LOG_PERIPH      RCC_APB_PERIPH_UART2
#define LOG_GPIO        GPIOA
#define LOG_PERIPH_GPIO RCC_APB_PERIPH_GPIO
#define LOG_TX_PIN      GPIO_PIN_1
#define LOG_RX_PIN      GPIO_PIN_3
#define LOG_ALT         GPIO_AF4
#else

#define LOG_UARTx       UART1
#define LOG_PERIPH      RCC_APB_PERIPH_UART1
#define LOG_GPIO        GPIOA
#define LOG_PERIPH_GPIO RCC_APB_PERIPH_GPIO
#define LOG_TX_PIN      GPIO_PIN_9
#define LOG_RX_PIN      GPIO_PIN_10	
#define LOG_ALT         GPIO_AF4


#endif
//#define YZ_B_PIN             GPIO_PIN_11 
//#define YZ_B_PORT            GPIOA

//#define B_HOLL_DIR_PIN       GPIO_PIN_9
//#define B_HOLL_DIR_PORT      GPIOA

//#define B_HOLL_NUM_PIN       GPIO_PIN_8
//#define B_HOLL_NUM_PORT      GPIOA

#define HOLL_DIR_PIN         GPIO_PIN_3
#define HOLL_DIR_PORT        GPIOB

#define HOLL_NUM_PIN         GPIO_PIN_4
#define HOLL_NUM_PORT        GPIOB

#define UP_LIMIT_PIN         GPIO_PIN_4
#define UP_LIMIT_PORT        GPIOA

#define DOWN_LIMIT_PIN       GPIO_PIN_5
#define DOWN_LIMIT_PORT      GPIOA

#define B_UP_LIMIT_PIN       GPIO_PIN_4
#define B_UP_LIMIT_PORT      GPIOB 

#define B_DOWN_LIMIT_PIN     GPIO_PIN_5
#define B_DOWN_LIMIT_PORT    GPIOB 

#define MOTOR_DN_PIN         GPIO_PIN_4
#define MOTOR_UP_PIN         GPIO_PIN_5

#define B_MOTOR_DN_PIN       GPIO_PIN_6
#define B_MOTOR_UP_PIN       GPIO_PIN_7

#define ZERO_CROSS_PIN       GPIO_PIN_12
#define ZERO_CROSS_PORT      GPIOB

#define PCBA_SELFTEST_PIN       GPIO_PIN_12
#define PCBA_SELFTEST_PORT      GPIOA

#define BUZZ_PIN       GPIO_PIN_8
#define BUZZ           GPIOA


#define LED_BLUE_ON       GPIO_SetBits(GPIOB, GPIO_PIN_5);
#define LED_BLUE_OFF      GPIO_ResetBits(GPIOB, GPIO_PIN_5);
#define LED_BLUE_TOGGLE   GPIO_TogglePin(GPIOB, GPIO_PIN_5);

#define LED_ORANGE_ON       GPIO_SetBits(GPIOB, GPIO_PIN_7);
#define LED_ORANGE_OFF      GPIO_ResetBits(GPIOB, GPIO_PIN_7);
#define LED_ORANGE_TOGGLE   GPIO_TogglePin(GPIOB, GPIO_PIN_7);

// 蜂鸣器控制
#define BUZ_ON                              TIM_ConfigPrescaler(TIM_BUZZ, 1, TIM_PSC_RELOAD_MODE_IMMEDIATE);	    TIM_Enable(TIM4, ENABLE);  // AdvTimer5运行
#define BUZ_OFF                             TIM_Enable(TIM_BUZZ, DISABLE);  
#define BuzVcc_ON                           GPIO_SetBits (BUZZ, BUZZ_PIN);
#define BuzVcc_OFF                          GPIO_ResetBits (BUZZ, BUZZ_PIN);
#define BuzVcc_TOGGLE                       GPIO_TogglePin(BUZZ, BUZZ_PIN);
#define BUZ_FRE_ON do { \
   TIM_BUZZ->AR = Buz->BzFre;  \
   TIM_BUZZ->CCDAT3 = Buz->BzFre/2;  \
} while(0)
#define BUZ_FRE_OFF do { \
    TIM_BUZZ->CCDAT3=0;   \
    TIM_BUZZ->CCDAT3=0;     \
} while(0)


// 强制输出：通道2 为高电平，通道3 为低电平  
#define PWM_FORCE_CH2_HIGH_CH3_LOW() \
do { \
    TIM_OUTPWM->CCMOD2 &= ~(0xE0 | (0xE0 << 8)); \
    TIM_OUTPWM->CCMOD2 |= (TIM_FORCED_ACTION_ACTIVE) | (TIM_FORCED_ACTION_INACTIVE << 8); \
} while(0)


// 宏功能：通道3 强制高(<<8) + 通道2 强制低
#define PWM_FORCE_CH3_HIGH_CH2_LOW() \
do { \
    /* 清空 CH3 控制位 */ \
    TIM_OUTPWM->CCMOD2 &= ~(0xE0 << 8); \
    /* CH3 强制高 | CH2 强制低 */ \
    TIM_OUTPWM->CCMOD2 |= (TIM_FORCED_ACTION_ACTIVE << 8) | TIM_FORCED_ACTION_INACTIVE; \
} while(0)
 

/********************************输入状态读取宏********************************/
#define PCBA_SELFTEST        GPIO_ReadInputDataBit(PCBA_SELFTEST_PORT, PCBA_SELFTEST_PIN)
#define YZ                   GPIO_ReadInputDataBit(YZ_PORT, YZ_PIN) 

#define CLEAR_HOLL_GetIrq    EXTI_ClrStatusFlag(EXTI_LINE5)
#define GetIrq_HOLL          EXTI_GetStatusFlag(EXTI_LINE5)
#define HOLL_DIR             GPIO_ReadInputDataBit(HOLL_DIR_PORT, HOLL_DIR_PIN)  //if(HOLL_DIR==1) 高电平触发为1   
#define HOLL_NUM             GPIO_ReadInputDataBit(HOLL_NUM_PORT, HOLL_NUM_PIN)
#define ZERO_CROSS           GPIO_ReadInputDataBit(ZERO_CROSS_PORT, ZERO_CROSS_PIN)

#define UP_LIMIT             GPIO_ReadInputDataBit(UP_LIMIT_PORT, UP_LIMIT_PIN)
#define DOWN_LIMIT           GPIO_ReadInputDataBit(DOWN_LIMIT_PORT, DOWN_LIMIT_PIN)
#define B_UP_LIMIT           GPIO_ReadInputDataBit(B_UP_LIMIT_PORT, B_UP_LIMIT_PIN)
#define B_DOWN_LIMIT         GPIO_ReadInputDataBit(B_DOWN_LIMIT_PORT, B_DOWN_LIMIT_PIN)

/********************************输出控制宏********************************/
#define MOTOR_DN_IO(value)   ((value) ? (TIM3->CCDAT3 = 3200) : (TIM3->CCDAT3 = 0))
#define MOTOR_UP_IO(value)   ((value) ? (TIM3->CCDAT4 = 3200) : (TIM3->CCDAT4 =0))

#define MOTOR_CLOSE_IO do { \
   TIM_SET_OC4_HIGH(TIM_OUTPWM) ; \
   TIM_SET_OC3_HIGH(TIM_OUTPWM) ;\
} while(0)

#define B_MOTOR_DN_IO(value) ((value) ? GPIO_SetBits(GPIOA, B_MOTOR_DN_PIN) : GPIO_ResetBits(GPIOA, B_MOTOR_DN_PIN))
#define B_MOTOR_UP_IO(value) ((value) ? GPIO_SetBits(GPIOA, B_MOTOR_UP_PIN) : GPIO_ResetBits(GPIOA, B_MOTOR_UP_PIN))

//#define LED24V_IO(value)     ((value) ? GPIO_SetBits(GpioPortB, GpioPin13) : Gpio_ClrIO(GpioPortB, GpioPin13))
//#define LED24V_IO2(value)    ((value) ? GPIO_SetBits(GpioPortC, GpioPin13) : Gpio_ClrIO(GpioPortC, GpioPin13))

/********************************LED灯光控制********************************/
#define LIGHT_WHITE_VALUE         TIM3->CCDAT1 
#define LIGHT_YELLOW_VALUE        TIM3->CCDAT2 
#define LED_LIGHT_OFF do { \
    GPIO_ResetBits(GPIOA, GPIO_PIN_15);   \
    TIM3->CCDAT1=0;\
	  TIM3->CCDAT2 =0;\
} while(0)

#define LED_LIGHT_ON do { \
    GPIO_SetBits(GPIOA, GPIO_PIN_15); \
    \
} while(0)


/********************************时间参数定义********************************/
#define TICKS_INTERVAL                       5   // ms
#define DEBOUNCE_TICKS                       3   // MAX 8
#define SHORT_TICKS                          (300 / TICKS_INTERVAL)
#define LONG_TICKS                           (10000 / TICKS_INTERVAL)  // 10s
#define QUEUE_BUF_DEEP                       256
#define RECEIVE_DATA_PACKET_INTERVAL_TIME    20  // 大于50ms判定为新数据包 步长1ms

/********************************行程参数********************************/
#define UP_POSITION_VALUE       1000
#define DOWN_POSITION_VALUE     2850
#define STROKE_TOTAL            1850    // 1.3m
#define UP_ELE_VALUE            1000
#define ONE_PERCENT_PULSE       (float)(STROKE_TOTAL/100)  // 百分之一对应霍尔数
#define WEIGHT_C0MPENSATION      10  //重量补偿
/********************************AD转换参数********************************/
#define AD_BUFF_DIP             11     
#define AD_AVERAGE_NUM          10     // 值越大越难触发
#define UPTREND_NUM             4      // 1s内上升趋势次数 值越大越难触发
#define AD_DIFF_NUM             10      // AD最大值相差值
/********************************照明参数********************************/
#define LIGHT_VALUE             16
#define A_OPTIMAL_DRY_POSITION  15
#define B_OPTIMAL_DRY_POSITION  60

/********************************功能选择AD值********************************/
#define OFFSET                  5  // 判断窗口AD值
#define FUNCTION_L_D_AD         (1.0/11*255)+OFFSET      // 照明消毒
#define FUNCTION_L_F_AD         (2.0/12*255)+OFFSET      // 照明风干
#define FUNCTION_L_H_AD         (3.0/13*255)+OFFSET      // 照明烘干风干
#define FUNCTION_L_D_F_AD       (4.3/14.3*255)+OFFSET    // 照明消毒风干
#define FUNCTION_L_D_F_H_AD     (5.6/15.6*255)+OFFSET    // 照明消毒风干烘干

#define FUNCTION_L              0X00    // 照明
#define FUNCTION_L_D            0X01    // 照明消毒
#define FUNCTION_L_F            0X02    // 照明风干
#define FUNCTION_L_H            0X03    // 照明烘干风干
#define FUNCTION_L_D_F          0X04    // 照明消毒风干
#define FUNCTION_L_D_F_H        0X05    // 照明消毒风干烘干

/********************************电机保护参数********************************/
#define OVER_WIGHT_REAL_CURRENT             (5)  // 真实电流值,默认每根杆超重21kg  1.72 17.5kg    // 采样电流值  1.61
#define OVER_WIGHT_CURRENT                  (float)(OVER_WIGHT_REAL_CURRENT*1.0717-0.1581) 
#define MOTOR_OVER_WEIGHT_TIEM              60     // 超重判定持续时间,步长10ms
#define OVER_WIGHT_CURRENT_ADVALUE_OFFSET   8      // 进入超重电流后的回滞AD值
#define OVER_WIGHT_CURRENT_ADVALUE_LIMIT    350    // 进入二级限位超重值 527-350=177
#define MOTOR_MAX_CIRCLE                    600    // 最多存6000次电机周期

#define MOTOR_ERR_CURRENT                   (4)     // 判断堵转电流
#define MOTOR_ERR_CURRENT_ADVALUE           (float)(MOTOR_ERR_CURRENT*0.1/3.6*4096)  // 堵转电流对应AD值 1,638.4
#define MOTOR_STALL_TIME                    10      // 堵转判定持续时间,步长10ms
#define MOTOR_ERR_CURRENT_ADVALUE_OFFSET    60      // 进入堵转判断后，取消堵转的回滞AD值

/********************************Flash存储参数********************************/
#define FDL_DATA_BLOCK1   512/sizeof(Flash_Data)  
#define FDL_DATA_BLOCK2   512/sizeof(Ell_Data) 
#define FDL_ONCE_MAX_WRITE_NUM             FDL_DATA_BLOCK1      // block0一块flash存储次数
#define FDL_BLOCK1_ONCE_MAX_WRITE_NUM      FDL_DATA_BLOCK2      // block1一块flash存储次数
#define FDL_Block0                         0x0800FC00 // FLASH_DATE
#define FDL_Block1                         0x0800FE00  // ELL_DATE

/********************************时间常量定义********************************/
#define ZERO_CROSSING_NO_SINGL_TIME         50     // 判定无过零信号时间 步长1ms
#define POWER_ON_ENABLE_STUDY_TIME          10000  // 上电10s内允许遥控器学习，步长1ms
#define EN_ADD_CODE_TIME                    5000   // 允许添码操作时间，步长1ms
#define Light_KEY_SPACE_TIME                3000   // 步长1ms
#define MOTOR_DIR_CHANGE_DELAY              1000   // 电机换向延时时间，步长1ms
#define MOTOR_DIR_CHANGE_DELAY1             200    // 轻抬上升后电机上升延时时间，步长1ms

#define ALARM_YZ_BEEP_NUM                   3      // 蜂鸣器报警重复次数
#define ALARM_OVER_WEIGHT_BEEP_NUM          3      // 蜂鸣器报警重复次数
#define ALARM_MOTOR_BEEP_NUM                3      // 蜂鸣器报警重复次数
#define ALARM_BEEP_CYCLE_TIME               5000   // 报警周期时间 步长1ms

#define DIS_JUDGE_MOTOR_OVER_WEIGHT_TIME    450   // 电机开始上升屏蔽超重检测时间，步长1ms
#define DIS_JUDGE_MOTOR_UP_OBS_TIME         300    // 电机开始上升屏蔽遇障碍物检测，步长1ms

#define WAIT_CONFIG_NETTIME                 17700  // 等待配网30min后橙灯亮起
#define LED_BLUE_TWINKLE_TIME               1800   // 闪烁持续时间 步长100ms
#define LED_BLUE_ON_TIME                    1800   // 长亮持续时间 步长100ms
#define LED_ORG_ON_TIME                     1800   // 长亮持续时间 步长100ms



#define LED_ORG_TWINKLE_TIME                1800//闪烁持续时间 步长100ms


#define LED_TWINKLE_ON_TIME                 1      // 闪烁亮灯时间 步长100ms
#define LED_TWINKLE_TIME                    5      // 闪烁周期时间 步长100ms

#define LIMIT_SIGNAL_FITER_TIME             50    //20     // 机械限位及非遇阻检测滤波时间 步长1ms
#define HOLL_PULSE_FILTER_TIEM              1      // 霍尔信号滤波时间 步长1ms

/********************************状态定义********************************/
// 电机状态
#define MOTOR_SUSPEND_STATE                 0X00   // 暂停
#define MOTOR_UP_STATE                      0X01   // 上升状态
#define MOTOR_DOWN_STATE                    0X02   // 下降
#define MOTOR_UP_STOP_STATE                 0X03   // 上限位停止状态
#define MOTOR_DOWN_STOP_STATE               0X04   // 下限位停止状态
#define MOTOR_YZ_STATE                      0X05   // 遇阻状态
#define MOTOR_OVER_STATE                    0X06   // 超重状态

// 提示音状态
#define BEEP_OFF                            0X00
#define BEEP_ON                             0X01

// 照明状态
#define LIGHT_OFF                           0X00
#define LIGHT_ON                            0X01

#define CONVERGENT_OFF                      0X00
#define CONVERGENT_ON                       0X01

// 遇阻信号判断时间定义
#define ENCOUNTER_Obstacle_TIME1            10     // 400ms,步长10ms
#define ENCOUNTER_Obstacle_TIME2            300    // 3s,步长10ms
#define ENCOUNTER_Obstacle_TIME3            20     // 步长10ms

// 灯渐亮步长
#define LIGHT_UP_STEP                       1600   // 3s渐亮

// 网络状态
#define NET_OFFLINE                         0
#define NET_UAP                             1
#define NET_LOCAL                           2
#define NET_CLOUD                           3
#define NET_UNPROV                          4
#define NET_UPDATING                        5

// 指示灯状态
#define LED_ORG_STATE_OFF                   0
#define LED_ORG_STATE_TWINK                 1
#define LED_ORG_STATE_ON                    2

#define LED_BLUE_STATE_OFF                  0
#define LED_BLUE_STATE_TWINK                1
#define LED_BLUE_STATE_ON                   2

/********************************Flash配置********************************/
#define DATA_FLASH_BLOCK_COUNTER            (2)
#define DATA_FLASH_BLOCK_SIZE               (512)
#define DATA_FLASH_SIZE                     (DATA_FLASH_BLOCK_COUNTER * DATA_FLASH_BLOCK_SIZE)
#define DATA_FLASH_ADDRESS_MAX              (DATA_FLASH_SIZE - 1)

#define FDL_FRQ                             (32)   // 时钟32
#define FDL_VOL                             (01)
#define BLOCK_ZERO                           0      //
#define BLOCK_ONE                            1      // 行程数据及运行次数

/********************************命令定义********************************/
#define MOTOR_UP_CMD     0X11
#define MOTOR_DOWN_CMD   0X33
#define MOTOR_STOP_CMD   0X55
#define LIGHT_CMD        0X0F
#define STUDY_CMD        0X79
 
/********************************433射频参数********************************/
#define OVEREIGHT_CURRENT_CMD              0XFE   // 超重电流校正命令
#define Capture_433KEY_UP_JUDGE_TIME       45     // 判断遥控器松手，步长5ms
#define Capture_433KEY_NONE                0X00
#define Capture_433KEY_PRESS               0X01
#define Capture_433KEY_UP                  0X02
#define Capture_433KEY_PRESS_LONG          0X03
#define RECEIVE_433_DISABLE_TIME           300    // 屏蔽接收时间

// 捕获码状态
#define CAPTURE_NO_DATA                    0X00   // 无数据
#define CAPTURE_HEAD_HIGH                  0X01   // 引导码高电平
#define CAPTURE_HEAD_LOW                   0X02   // 引导码低电平
#define CAPTURE_FIXED_CODE                 0X03   // 识别码
#define CAPTURE_ROLL_CODE                  0X04   // 命令码
#define CAPTURE_COMMANG_END                0X05   // 命令码
#define ALL_CODE_LEVELS                    129    // 最后1bit命令码只测高电平
#define FIXED_CODE_LEVELS                  81

// 引导电平冗余
#define HEAD_HIGH_MIN_TIME                  3800
#define HEAD_HIGH_MAX_TIME                  5000
#define HEAD_LOW_MIN_TIME                  (1200)
#define HEAD_LOW_MAX_TIME                  (1900)

// 数据电平冗余
#define _800US_MIN_TIME                    (650)
#define _800US_MAX_TIME                    (950)
#define _400US_MIN_TIME                    (200)
#define _400US_MAX_TIME                    (600)
#define _433_DATA                          GPIO_ReadInputDataBit(GPIOB,GPIO_PIN_6)
#define INTERFERE_PLUSE_WIDTH              200

/********************************蜂鸣器配置********************************/
// 频率定义
#define FREQ_NO                            0
#define FREQ_2300                          6955UL*2   // 3478//6957
#define FREQ_2600                          6152UL*2   // 3077//6154
#define FREQ_2900                          5516UL*2   // 2900Hz
#define FREQ_2000                          7999UL*2   // 2000Hz
#define FREQ_1238                          25886UL    // 1238Hz
#define FREQ_1288                          12421UL*2  // 1288Hz
#define FREQ_1342                          11921UL*2  // 1342Hz
#define FREQ_NULL                          10000  // 作为间隔等待，当个标志

#define SET_DOWN_LIMIT   0X99

// 蜂鸣器类型
#define M_BZ                              0    
#define M_BZ_DI                           1	 
#define M_BZ_KEY                          2	 
#define M_BZ_ON                           3	 
#define M_BZ_OFF                          4	 
#define M_YZ_ONE                          5	 
#define M_CZ_TWO                          6    
#define M_MOR_THREE                       7	
#define M_FUN_ON                          8	
#define M_FUN_OFF                         9	

// 蜂鸣音效
#define BEEP_PWM_OFF                      0 
#define SHORT_BEEP                        1
#define LONG_BEEP                         2 
#define RISE_TONE_BEEP                    3 
#define FALL_TONE_BEEP                    4
#define LONG_BEEP_TWO                     5
#define LONG_BEEP_THREE                   6
#define RISE_TONE_BEEP_MODE1              7 
#define FALL_TONE_BEEP_MODE1              8

// 功能对应的蜂鸣音效
#define POWER_ON_BEEP                     RISE_TONE_BEEP_MODE1
#define CODE_MATCH_BEEP                   RISE_TONE_BEEP_MODE1
#define WIFI_RESET_BEEP                   RISE_TONE_BEEP_MODE1
#define ADD_MATCH_CODE_BEEP               RISE_TONE_BEEP_MODE1
#define LIGHT_ON_BEEP                     RISE_TONE_BEEP
#define LIGHT_OFF_BEEP                    FALL_TONE_BEEP
#define FUN_ON_BEEP                       RISE_TONE_BEEP
#define FUN_OFF_BEEP                      FALL_TONE_BEEP
#define GENTLY_LIFT_MOTOR_UP_BEEP         RISE_TONE_BEEP_MODE1
#define MOTOR_UP_BEEP                     RISE_TONE_BEEP  
#define MOTOR_DOWN_BEEP                   RISE_TONE_BEEP  
#define MOTOR_STOP_BEEP                   LONG_BEEP
#define YZ_MOTOR_STOP_BEEP                LONG_BEEP
#define YZ_ALARM_BEEP                     LONG_BEEP
#define OVER_WIGHT_ALARM_BEEP             LONG_BEEP_TWO
#define MOTOR_ERR_ALARM_BEEP              LONG_BEEP_THREE
#define MOTOR_ERR_ALARM_BEEP           (10)
/********************************PWM电机控制********************************/
#define PWM_STOP                          0
#define PWM_DOWN                          1 
#define PWM_UP                            2 

#define PWM_MAX_VALUE                     1600        // PWM最大占空比值因子

#define PWM_MIN_COMPARE                   5           // 最小比较值阈值
#define PWM_MAX_COMPARE                   100         // 最大比较值阈值 100*5ms=500ms
#define PWM_START_SPEED                   2           // PWM启动速度因子
#define PWM_STOP_SPEED                    2           // PWM停止速度因子

#define PWM_WIFI_VALUE                    PULSES_PER_MS*PWM_MAX_COMPARE*5  // wifi定点偏移量

#define PWM_DUTY                          PWM_BASE_VALUE + ((PWM_MAX_VALUE-PWM_BASE_VALUE) / (PWM_MAX_COMPARE *PWM_RAMP_RATE)) *PWM_RAMP_RATE * u16CHxACompare
#define PWM_DUTY_B                        PWM_BASE_VALUE + ((PWM_MAX_VALUE-PWM_BASE_VALUE) / (PWM_MAX_COMPARE *PWM_RAMP_RATE)) *PWM_RAMP_RATE * u16CHxbCompare

#define Motor_PWM_DN_MAX do { \
   	TIM_SetCmp3(TIM_OUTPWM, 0);\
    TIM_SetCmp4(TIM_OUTPWM, 3200);\
} while(0)
#define Motor_PWM_UP_MAX do { \
   	TIM_SetCmp3(TIM_OUTPWM, 3200);  \
    TIM_SetCmp4(TIM_OUTPWM, 0);\
} while(0)

#define MotorA_PWM_Stop                  
#define MotorA_PWM_Start                  
#define MotorA_PWM_DN(a)   do { \
   	  TIM_SetCmp3(TIM_OUTPWM, 3200);	 \
      TIM_SetCmp4(TIM_OUTPWM, a);\
} while(0)                    
#define MotorA_PWM_UP(a)   do { \
   	  TIM_SetCmp3(TIM_OUTPWM, a);	 \
      TIM_SetCmp4(TIM_OUTPWM, 3200);\
} while(0)  


// 通道1
#define TIM_SET_OC1_LOW(TIM_OUTPWM)    (TIM_OUTPWM->CCMOD1 &= ~(0x7 << 4))
#define TIM_SET_OC1_HIGH(TIM_OUTPWM)   do{ TIM_OUTPWM->CCMOD1 &= ~(0x7 << 4); TIM_OUTPWM->CCMOD1 |= (0x1 << 4); }while(0)
#define TIM_SET_OC1_PWM(TIM_OUTPWM)    do{ TIM_OUTPWM->CCMOD1 &= ~(0x7 << 4); TIM_OUTPWM->CCMOD1 |= (0x6 << 4); }while(0)

// 通道2
#define TIM_SET_OC2_LOW(TIM_OUTPWM)    (TIM_OUTPWM->CCMOD1 &= ~(0x7 << 12))
#define TIM_SET_OC2_HIGH(TIM_OUTPWM)   do{ TIM_OUTPWM->CCMOD1 &= ~(0x7 << 12); TIM_OUTPWM->CCMOD1 |= (0x1 << 12); }while(0)
#define TIM_SET_OC2_PWM(TIM_OUTPWM)    do{ TIM_OUTPWM->CCMOD1 &= ~(0x7 << 12); TIM_OUTPWM->CCMOD1 |= (0x6 << 12); }while(0)

// 通道3
#define TIM_SET_OC3_LOW(TIM_OUTPWM)    (TIM_OUTPWM->CCMOD2 &= ~(0x7 << 4))
#define TIM_SET_OC3_HIGH(TIM_OUTPWM)   do{ TIM_OUTPWM->CCMOD2 &= ~(0x7 << 4); TIM_OUTPWM->CCMOD2 |= (0x1 << 4); }while(0)
#define TIM_SET_OC3_PWM(TIM_OUTPWM)    do{ TIM_OUTPWM->CCMOD2 &= ~(0x7 << 4); TIM_OUTPWM->CCMOD2 |= (0x6 << 4); }while(0)

// 通道4
#define TIM_SET_OC4_LOW(TIM_OUTPWM)    (TIM_OUTPWM->CCMOD2 &= ~(0x7 << 12))
#define TIM_SET_OC4_HIGH(TIM_OUTPWM)   do{ TIM_OUTPWM->CCMOD2 &= ~(0x7 << 12); TIM_OUTPWM->CCMOD2 |= (0x1 << 12); }while(0)
#define TIM_SET_OC4_PWM(TIM_OUTPWM)    do{ TIM_OUTPWM->CCMOD2 &= ~(0x7 << 12); TIM_OUTPWM->CCMOD2 |= (0x6 << 12); }while(0)



/********************************其他定义********************************/
#define REG_EXT                          extern volatile
#define _433_COMMAND                     1
#define APP_COMMAND                      0


extern void HAL_TIM_CNT_IRQHandler_BACK(void);
extern void HAL_TIM1_CC_IRQHandler_BACK(void);
extern void HAL_EXIT_IRQHandler_BACK(void);
extern void HAL_ADC_IRQHandler_BACK(void);
#endif

#ifdef __cplusplus
}
#endif
