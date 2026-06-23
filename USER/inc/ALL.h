#ifndef __ALL_H__
#define __ALL_H__
#ifdef __cplusplus
extern "C" {
#endif
#include "main.h"
typedef struct
{
	uint8_t  Capture_433key;
	uint8_t  Capture_433key_State;
	uint16_t Capture_433key_Judgetime;     // 按键判断时长
	uint16_t Capture_433key_Cmd;

	uint8_t  Capture_State;                // 状态
	uint8_t  Capture_Pulse_Wide_Sequence;  // 脉冲宽度序列号
	uint16_t Capture_Pulse_Wide_value[140];// 脉冲宽度值数组
	uint16_t Command_data;                 // 8位命令码
	uint32_t Identify_data;                // 24位识别码 + 8位命令码
	uint32_t Secret_Key_data;              // 32位密钥
}CAPTURE_COMMAND;
extern CAPTURE_COMMAND CAPTURE_COMMAND1;

typedef enum {
    PRESS_DOWN = 0,
    PRESS_UP,
    PRESS_REPEAT,
    SINGLE_CLICK,
    DOUBLE_CLICK,
    THREE_CLICK,
    FORE_CLICK,          // 注意：此处拼写错误，应为FOUR_CLICK
    FIVE_CLICK,
    SIX_CLICK,
    SEVEN_CLICK,
    LONG_RRESS_START,    // 注意：此处拼写错误，应为LONG_PRESS_START
    LONG_PRESS_HOLD,
    LONG_PRESS_UP,
    NONE_PRESS
} PressEvent;

extern PressEvent PressEven;  // 按键事件变量
// 按键结构体类型定义
typedef struct {
    uint16_t ticks;
    uint8_t  repeat       : 4;
    uint8_t  event        : 4;
    uint8_t  state        : 4;
    uint8_t  debounce_cnt : 4;
    uint8_t  button_level;
  uint8_t  (*hal_button_Level)(void);
} button;


typedef struct
{
    uint8_t Encounter_Obstacle_beepnum ; // 遇障碍物报警蜂鸣次数
    uint8_t Over_Wight_beepnum         ; // 超重报警蜂鸣次数
    uint8_t Motor_Err_beepnum          ; // 电机故障报警蜂鸣次数
    uint8_t Holl_Pluse_Filter_time     ; // 霍尔脉冲滤波时间
    uint8_t return_to_up_time          ; // 回到上限位滤波时间
    uint8_t return_to_down_time        ; // 回到下限位滤波时间

    uint8_t Enter_Pcba_test_time       ; // 进入PCBA测试滤波时间
    uint8_t Uart_Receive_Interval_time ; // 串口接收数据包间隔
    uint8_t Uart2_Receive_Interval_time; // 串口2接收数据包间隔
    uint8_t _150ms_Count;               // 150ms计数
    uint8_t Zero_crossing_no_single_time;
    uint8_t Motor_Err_time;            // 电机堵转连续触发禁止时间

    uint8_t time5ms;
    uint8_t time10ms;
    uint8_t time100ms;

    uint16_t time150ms;
    uint16_t beep_off_5stime;          // 报警蜂鸣间隔重复时间
    uint16_t power_on_10stime;         // 上电10s
    uint16_t Light_key_space_time;     // 灯光按键间隔时间

    uint16_t time_5min;
    uint16_t  Test_speed_time;
    uint16_t Motor_Dir_Change_Delaytime;// 电机方向切换延时
    uint16_t Motor_Stop_Encounter_Obstacle_time;// 电机停止遇阻时间
    uint8_t  Motor_No_Encounter_Obstacle_time;   // 电机无遇阻时间
    uint16_t Del_Match_Code_time;      // 删除配对码有效时间,5s有效

    uint16_t Judge_Over_Wight_time;    // 电机超重判断滤波时间
    uint16_t Judge_Motor_Err_time;     // 电机堵转判断滤波时间
    uint16_t Motor_Start_Up_time;      // 电机开始启动时间

    uint16_t Motor_Run_time;           // I/O控制实际电机得电，当前设为1s保证继电器稳定
    uint16_t Up_Data_Delay_time;       // 电机上报停止后延迟上报告警

    uint32_t WIFI_enable_config_time;  // 模块配网等待时间

    uint16_t Overweight_Current_time;  // 校准超重电流时间

    uint16_t  Motor_Obs_Down_time;     // 电机遇障碍物下降时间

    uint16_t  Motor_Force_Down_time;   // 强制下降时间
       uint32_t   Enable_Add_Match_Code_time;// 配对时间
}TIME_SLICE;



extern volatile TIME_SLICE  Time;

typedef struct
{
    uint8_t   Function;
    uint16_t  ADvalue;
    uint16_t  AD_sumple_num;
    uint32_t  AD_sum;
}FUNCTION_SELECT;

extern volatile FUNCTION_SELECT  Function_select,Motor_Current,Motor_Current_B;

typedef struct
{
    uint8_t Power_Failure_flag                :1; // 检测到掉电标志位
    uint8_t Wifi_uap_time_over                :1;

    uint8_t AD0_Change_end                    :1; // AD0转换完成
    uint8_t AD19_Change_start                 :1; // AD通道切换到AD19

    uint8_t power_on_10stime_flag             :1;
    uint8_t beep_off_5stime_flag              :1;
    uint8_t time5ms_flag                      :1;
    uint8_t time10ms_flag                     :1;
    uint8_t time100ms_flag                    :1;
    uint8_t time150ms_flag                    :1;

    uint8_t Motor_run_now                     :1; // 电机正在运行
    uint8_t Motor_up_to_down                  :1; // 电机上升转下降
    uint8_t Motor_down_to_up                  :1; // 下降转上升
    uint8_t Motor_raise_up                    :1; // 电机抬升

    uint8_t Encounter_Obstacle_Motor_stop     :1; // 遇阻停止
    uint8_t Add_Match_Code_flag               :1; // 添加遥控器配对

    uint8_t Motor_Start_up    		      :1; // 电机开始启动
    uint8_t Enable_Test_Over_Wight_flag       :1; // 使能检测超重
    uint8_t Enable_Test_Motor_Obs_flag        :1; // 使能检测电机遇障碍物

    uint8_t Enter_Pcba_test_flag              :1; // 进入PCBA自检

    uint8_t _220AC_Power_Down_flag            :1; // 掉电标志

    uint8_t Already_save_data_flag            :1; // 已经保存一次数据

    uint8_t No_judge_electronic_limit         :1; // 不判断电子限位

    uint8_t Brightness_time_flag              :1; // 夜间时间标志

    uint8_t Brightness_change                 :1; // 夜间状态下的亮度切换

    uint8_t Run_to_set_position_flag          :1; // 运行到指定位置

    uint8_t Up_event_finalposition            :1; // 需要上报最终位置事件

    uint8_t Auto_light                        :1; // 自动开灯

    uint8_t Reset_model                       :1; // 防止遥控器配对设置完成后收到模块状态改变而多配对一个

    uint8_t Need_433_data_compensate          :1; // 433数据需要补偿

    uint8_t Overweight_current_correct_start  :1; // 开始校准超重

    uint8_t Motor_force_down                  :1; // 强制下降

    uint8_t Uart1_Err                         :1; // 接收wifi数据出错

    uint8_t Beep_open_now                     :1;

    uint8_t Up_limit                          :1;
    uint8_t Down_limit                        :1;
    uint8_t Yz                                :1;


		uint8_t mcu_wifi_mode;
}FLAG;


typedef union
{
    uint8_t data;
    struct
    {
		uint8_t  Encounter_Obstacle     :1; // 遇障碍物
		uint8_t  Over_Wight             :1;
		uint8_t  Motor_Err              :1;
		uint8_t  Motor_Obs		        :1;
    }err_bit;
}ERR;
extern  ERR Err,Err_B;

typedef struct
{
//siid=2
    uint8_t Err_Byte;            // piid=1 故障参数: 可读可上报; Value 0: 正常 1: 遇阻 2: 超重 4: 电机故障
    uint8_t Motor_State_Byte;    // piid=2 电机状态参数: 可读可上报; Value 0: 下限位停止 1: 上升 2: 下降 3: 暂停 4: 上限位停止
    				 // piid=5 电机控制参数: 写操作
    uint8_t Motor_Position;      // piid=4 电机当前位置参数: 只读, 上报 (0-100%)
    uint8_t set_position;        // piid=5 运行到指定位置参数: 只读, 可写可上报
    uint8_t electronic_up_limit; // piid=6 电子上限位参数: 只读, 可写可上报 value 0-49%
    uint8_t electronic_down_limit;//piid=7 电子下限位参数: 只读, 可写可上报 value 51-100%

    uint8_t convergent;          // bool piid=8 一键收拢
    uint16_t current_speed;      // piid=9  上报: 1-500对应0.1-50, 实际为Float
    uint8_t preset_position;     // piid=10 一键晾干预置位置

//siid=3
    uint8_t Light_State_Byte;    // bool piid=1 灯光开关参数: 只读, 可写可上报
    uint8_t Light_Bright;        // piid=2 灯光亮度参数: 只读, 可写可上报 value: 0-100%
    uint16_t  Light_colour_temperature; // piid=4 色温 3000-6500 档位5
    uint8_t nightlight_switch;   // piid=4 夜灯开关
    uint8_t brightness;          // piid=5 夜灯亮度参数
//siid=5,
    uint8_t Beep_Enable;         // bool piid=1 提示音开关参数: 只读, 可写可上报 value: 0=关闭 1=开启
//siid=6
    uint8_t B_Err_Byte;          // B端故障
    uint8_t B_Motor_State_Byte;  // piid=3, B端电机状态

    uint8_t B_Motor_Position;
    uint8_t B_set_position;
    uint8_t B_electronic_down_limit;
    uint8_t B_electronic_up_limit;

    uint8_t B_preset_position;
    uint8_t B_convergent;
    uint16_t B_current_speed;
   //siid=7
    uint8_t Err_Byte_all;        // 双端故障
    uint8_t convergent_all;      // 双端收拢 piid=16, bool
    uint8_t set_position_all;
    uint8_t lightmode;
	// 夜灯开始时间
    uint8_t start_time_hour;     // piid=9 夜灯开启时间小时参数
    uint8_t start_time_min;      // piid=9 夜灯开启时间分钟参数
    uint8_t end_time_hour;       // piid=9 夜灯关闭时间小时参数
    uint8_t end_time_min;        // piid=9 夜灯关闭时间分钟参数

    uint8_t  Control_open;
    uint8_t  Control_num;
}DEVICE_STATE_DATA;              // 与app通信数据结构体

extern volatile DEVICE_STATE_DATA   Device_State_Data,Up_State_Data; // 设备上报状态
extern  uint8_t  FDL_Block0_Save_Times,FDL_Block1_Save_Times;       // flash_data存储次数
typedef struct
{
    uint8_t    Max_Light_Value    ; // 最大亮度

    uint8_t    Light_Bright_Value :7; // 夜灯亮度 0=日光 1=夜灯
    uint8_t    Light_Bright_Enable:1; // 启用夜灯: 1=有效

    uint8_t    Light_open_hour_time:6; // 夜灯开启小时
    uint8_t    Device_motor_state   :2; // 电机当前状态(低位)

    uint8_t    Light_open_min_time:6;  // 夜灯开启分钟
    uint8_t    Device_light_state   :2; // 电机当前灯光状态

    uint8_t    Light_close_hour_time:6;// 夜灯关闭小时
    uint8_t    Device_updata       :2;  // 升级标志位: 0=有效

    uint8_t    Light_close_min_time:6; // 夜灯关闭分钟
    uint8_t    Device_motor_state1   :2; // 电机当前状态(高位)

    uint8_t    Remote_Control_Num   :7; // 存储遥控器数量
    uint8_t    Beep_open            :1; // 提示音开关: 1=开启

    uint8_t    Light_pwm_value;        // 灯光亮度百分比, 用于判断是否超过之前是否进入夜灯模式
    uint16_t   Light_CT_value;         // 灯光色温值
    uint8_t    Control_open;

    uint8_t   Electronic_Down_Limit;   // 电子下限位, 百分比
    uint8_t   Overweight_Correct;      // 125对应0, 126对应默认值+0.01A, 124对应默认值-0.01A, 以此类推
    uint8_t   Uptrend_Num_Correct;

    uint16_t   Electronic_Up_Limit  :15;// 电子上限位, 百分比
    uint16_t   Quite_updata         :1; // 静默升级标志位: 0=有效

    uint32_t  Remote_Control_Key[3];    // 遥控器密钥
    uint32_t  Remote_Control_Id[3];     // 遥控器Id

    uint32_t  reserve;                  // 预留4个字节
    uint16_t  Check;
}FDL_DATA;                               // 44 bytes
extern volatile FDL_DATA  Flash_Data;
extern uint16_t  Synchronous_Count_value[3]; // 同步计数值

typedef struct
{
    uint16_t  Motor_Current_Position;   // 电机当前位置对应脉冲数
    uint16_t  Motor_Run_Circle;         // 电机循环运行次数(10圈为单位)

    uint16_t  B_Motor_Current_Position; // B电机当前位置对应脉冲数
    uint16_t  B_Motor_Run_Circle;       // B电机循环运行次数(10圈为单位)
}ELL_DATA;
extern volatile ELL_DATA  Ell_Data;

typedef struct
{
	uint8_t  Light_Led_state;
	uint8_t  Light_Breathing_time;
	uint8_t  Led_Blue_state          :2; // 蓝灯显示状态
	uint8_t  Led_Orange_state        :2; // 橙灯显示状态
	uint8_t  Led_Twinkle_time        :4; // LED闪烁时间(0.1s亮, 0.4s灭)
	uint16_t Led_Blue_On_time;           // 蓝灯常亮时间, 3min后熄灭
	uint16_t Led_Orange_On_time;         // 橙灯闪烁时间, 30min后关闭
}LED;
extern  LED Led ;

typedef struct
{
	uint8_t rf433_test_flag: 	        1;
	uint8_t input_YZ_test_flag:         1;
	uint8_t input_uplimit_test_flag:    1;
	uint8_t input_downlimit_test_flag:  1;
	uint8_t wifi_test_flag:             1;
}PCBA_TEST;
extern  PCBA_TEST Pcba_test;

typedef struct                        // 超重值校准
{
	uint16_t Sample_num;              // 采样次数
	uint32_t Sample_sum;              // 采样值总和
	float Sample_ave;                 // 平均AD值
	uint16_t sample_current;
}OVERWEIGHT_CORRECT;
extern  OVERWEIGHT_CORRECT Overweight_Correct;


////============= BUZ Frequency ================
////2.3  2.6  2.9  Fpclk 24M
//#define   FREQ_NO     0
//#define   FREQ_2300   5217//3478//6957
//#define   FREQ_2600   4617//3077//6154
//#define   FREQ_2900   4140//2759//5517.
//
//#define   FREQ_2000   6000//4000//8000
//#define   FREQ_1238   9657//6437//12874   //12924
//#define   FREQ_1288   4000            //  3000//6186//12372   //12422
//#define   FREQ_1342   8907//5937//11873   //11923
//============= BUZ Kinds =====================
typedef	struct tBUZHX
{
	uint16_t BzFre;	   // Frequency
	uint8_t  T_BzO;    // Frequency time
	uint8_t  T_BzV;    // Vcc
}tBuzHx ;


// 设备状态结构体定义
typedef struct {
    uint8_t Poweron_Set_Model_step;  // 上电设置模式步骤
    uint8_t g_MiioRoundFlag;         // Miio轮询标志
    uint8_t g_MiioErrorFlag;         // Miio错误标志
} DeviceStatus_TypeDef;

// 声明全局结构体变量
//DeviceStatus_TypeDef g_DevStatus = {0, 0, 0};

extern DeviceStatus_TypeDef g_DevStatus;

// 结构体类型定义
typedef struct {
    const char *Up_Motor_Properties[5];
    const char *Up_Motor_Properties_B[5];
    const char *SentReast;
    const char *enter;
    const char *ModelCheck;
    const char *ModelCharCheck;
    const char *MCUversion;
    const char *ModelChar;
    const char *ModelPid;
    const char *MIIOuartarck;
    const char *MCUnet;
    const char *DownInstruct;
    const char *getnettime;
    const char *error5001;
    const char *error5002;
    const char *setresult;
    const char *properties;
    const char *upevent;
    const char *QueryMcuVersion;
    const char *Arcknone;
    const char *ArckOk;
    const char *offline;
    const char *local;
    const char *cloud;
    const char *uap;
    const char *unprov;
    const char *updating;
    const char *UnknownCommand;
    const char *WIFIoffline;
    const char *WIFIlocal;
    const char *WIFIcloud;
    const char *WIFIuap;
    const char *WIFIunprov;
    const char *WIFIupdating;
    const char *error;
    const char *poweroff;
    const char *SetProperties;
    const char *ActionImplement;
    const char *QueryOneState;
    const char *bootloard;
    const char *auto_on;
    const char *auto_off;
    const char *autoupdate;
    const char *forceupdate;
    const char *autoready;
    const char *autobusy;
} MiioConst_TypeDef;

// 外部声明
extern const MiioConst_TypeDef MiioConst;

//============= function call ==================

// ADC 相关变量
extern int16_t u32AdcRestult4;
extern int16_t  u32AdcRestult2;
extern uint8_t ADC_sample_num, min_serial_num, max_serial_num;
extern uint8_t ADC_sample_num_B, min_serial_num_B, max_serial_num_B;
extern uint8_t ADC_sample_num_yz, min_serial_num_yz, max_serial_num_yz;
extern uint8_t ADC_sample_num_yz_B, min_serial_num_yz_B, max_serial_num_yz_B;
extern uint16_t ADC_sample_max, ADC_sample_min, ADC_sample_max_yz, ADC_sample_min_yz;
extern uint16_t ADC_sample_max_B, ADC_sample_min_B, ADC_sample_max_yz_B, ADC_sample_min_yz_B;
extern uint16_t ADC_read_buf[AD_BUFF_DIP], ADC_read_buf_B[AD_BUFF_DIP];

// 按键相关变量
extern button key_set_key, capture_433key;
extern uint8_t key_value;

// 电机控制相关变量
extern uint8_t conform_motor_obs_num, conform_motor_obs_num2;
extern uint8_t A_obs_correction;

// 系统状态变量
extern uint8_t Self_Inspection_step;           // 自检步骤
extern volatile uint8_t Holl_Circle_num, Motor_Circle_num;
extern uint16_t Holl_pulse_temp;
extern volatile int Motor_Current_Position_pulse, Holl_test_speed_temp;

// 限位设置变量
extern uint16_t set_down_limit_value;
extern uint16_t set_up_limit_value;
extern uint16_t set_position_pulse;

// 灯光控制变量
extern uint16_t set_light_value;               // 灯光实时值
extern uint16_t brightness_target, TDR02_target_value, TDR03_target_value, brightness_temp;
extern  uint16_t Light_colour_temperature_gear;

// 状态及通信变量
extern uint8_t current_state;
extern uint8_t Net_state;
extern uint8_t Flag_TOP_ods;                   // 电机到了上限位一次标志位
extern uint8_t Flag_TOP_odsB;                  // B电机到了上限位一次标志位
extern uint16_t target_position;
extern uint32_t CPU_RUNNING;

// 通信相关变量
extern uint8_t send_busy_asr;

extern uint8_t send_busy;

extern __IO  uint16_t colour_temperature_temp;
extern  uint8_t Study_Cmd_Repeat,STOP_CMD_Repeat; // 学习命令重复计数


// 数据缓冲区
extern uint8_t uart1_rx_buf[256];              // wifi接收数据缓冲区
extern uint8_t uart1_tx_buf[256];              // 发送给wifi的数据缓冲区

extern  uint16_t UART_RX_Count ,Frame_length ;
// 消息常量
extern uint8_t Obs1[8];                        // 遇阻
extern uint8_t Obs2[8];                        // 遇阻
extern uint8_t OVER_WIGHT1[8];                 // 超重
extern uint8_t OVER_WIGHT2[8];                 // 超重
extern uint8_t Upper_limit_tip[8];
extern uint8_t Confirm_asr[8];

// 校准及计算变量
extern uint16_t crc;
extern float temp1;
extern float over_weight_current;
extern float addvalue_A, addvalue_B;

// 全局标志
extern FLAG Flag;

// 硬件自检函数声明
extern void PCBA_Self_Inspection(void);
extern void SetupBzhx(uint8_t MBz,uint8_t flag);
// 遥控器控制函数声明
extern void Record_PulseWidth(uint32_t g_tau0_ch7_width, CAPTURE_COMMAND *Capture_Command);
extern uint8_t analysis_capture_command(CAPTURE_COMMAND *Capture_Command, uint8_t power_on_10stime_flag);  // 处理遥控器命令
extern uint8_t judge_remote_control(uint32_t roll_code);
extern void capture_433key_judge_answer(button* handle, uint8_t power_on_10stime_flag);

// 遥控器命令结构体
extern CAPTURE_COMMAND Capture_Command;

//TIM0  1ms 定时器
extern void App_AdvTimerInit(uint16_t u16Period,  uint16_t u16CHB_PWMDuty);
extern void App_AdvTimerPortInit(void);
extern void App_Timer0PortCfg(void);
extern void App_Timer0Cfg(uint16_t u16Period);
extern void Run_to_position(void);
extern void Run_to_position_B(void);
extern void Action_deal(void);
extern void Return_Properties(void);           // 响应app查询属性
extern void Set_Properties (void);             // 响应app设置指令
extern uint8_t properties_up(void);            // 设备状态上报
extern void BootLoardDeal(void);
extern void App_portCfg(void);
extern void App_UartCfg(void);
extern void App_LpUartCfg(void);
extern void App_LpUartPortCfg(void);
extern void r_uart0_service(void);
extern unsigned char Queue_Read_Byte(void);
extern void r_uart0_receive_input(unsigned char value);
extern unsigned char get_queue_total_data(void);
extern void r_uart0_send_bytes(unsigned char *src_p,unsigned char dataNum);
extern void App_ClockCfg(void);
extern void r_uart0_protocol_init(void);
extern void _150ms_work(void);
extern void UpMCUversion(void);
extern void r_uart1_service(void);
extern void r_uart1_protocol_init(void);
extern void App_TimerLED2Cfg(uint16_t u16Period, uint16_t u16CHxACompare);
extern void r_uart1_send_bytes(unsigned char *src_p,unsigned char dataNum);
extern void App_Timer1PortCfg(void);          // Timer3 Port配置
extern void App_Timer1Cfg(void);
extern void App_Timer3PortCfg(void);
extern void App_Timer3Cfg(void);
extern void variable_init(void);
extern void _500ms_work(void);
extern void function_select(void);
extern void App_WdtInit(void);
extern void SetupBzhx(uint8_t MBz,uint8_t flag);
extern    uint8_t CmdBzhx(void);
extern void App_AdvTimerInit(uint16_t u16Period,  uint16_t u16CHB_PWMDuty);
extern void App_AdvTimerPortInit(void);
extern void App_GpioInit(void);
extern void App_SPIInit(void);
extern void App_UserKeyInit(void);
extern void App_LedInit(void);
extern void r_uart1_receive_input(unsigned char value);
extern void light_led_work(void);             // 呼吸灯指示效果
/****************FLASH*************************FLASH*********************FLASH**********************FLASH**********************FLASH***************/
extern void read_data(void);
extern void Cmd_Light(void);
extern uint8_t Read_FLASH_Data(uint32_t Addr);
extern void match_data_del(void);
extern void match_data_clear(void);
extern void save_data(void);
extern void save_current_position(void);
extern void App_LvdInit(void);
extern void App_LvdPortInit(void);
extern void _220AC_Power_down_save(void);     // 掉电保存
extern void limit_protect(void);
extern void limit_protect_B(void);
extern void data_handle(void);
extern unsigned char getBit(unsigned char source[], int n) ;
extern void RF_DEC(unsigned char *source) ;
extern void RLC(unsigned char *source, char c, char n);
extern void Keelop_KEY(unsigned long  key_write);
extern void PowerOn_ModelSet(void);
extern void led_work(void);
extern void variable_init(void);
extern void App_TimermotorPortCfg(void);
extern void App_TimermotorCfg(uint16_t u16Period, uint16_t u16CHxACompare, uint16_t u16CHxBCompare);
extern void alarm_signal_test_and_beep_B(void);
extern void alarm_signal_test_and_beep(void);

/*****************电机*******************//*****************电机*******************//*****************电机*******************//*****************电机*******************//*****************电机*******************/
extern void App_MORInit(void);
extern void App_Init(void);
extern void Cmd_Motor_Down(void);
extern void Cmd_Motor_Stop(void);
extern void light_work(uint8_t light_state);
extern void motor_work(uint8_t motor_state);
extern void motor_stop(void);
extern void motor_up(void);
extern void motor_down(void);
extern void Delay1ms(uint32_t nTime);
extern void Cmd_Motor_Up(void);
extern void Cmd_Motor_Up_Longpress(void);
extern void Motor_speed(void);
/*****************ADC*************//*****************ADC*************//*****************ADC*************//*****************ADC*************//*****************ADC*************/

extern void App_AdcJqrCfg(void);
extern void App_AdcPortInit(void);
extern void App_AdcInit(void);
extern uint16_t motor_current_to_advalue(float  circle);
extern void Ovreweight_current_correct(void);

extern void motor_stop(void);
extern void motor_up(void);
extern void motor_down(void);
extern void ModuleResetReq(void);             // 重置模块
extern void motor_work(uint8_t motor_state);
extern void light_work(uint8_t light_state);
extern uint32_t Read_FLASH_MUIData(uint32_t address, uint8_t* buffer, uint32_t length);
extern FLASH_STS Flash_WriteStruct(uint32_t address, const uint8_t* data, uint32_t length);
extern void read_data(void);
extern void PVD_Init(void);


extern void WDT_Init(void);
#endif




#ifdef __cplusplus
}
#endif
