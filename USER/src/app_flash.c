#include "ALL.h" 
#include "main.h"
uint8_t   FDL_Block0_Save_Times,FDL_Block1_Save_Times;
uint16_t  Synchronous_Count_value[3];//同步计数值
void match_data_clear(void);
/*读取存储数据*/
void read_data(void)
{
    uint16_t i = 0;

    /* 读取块0的Flash数据 */
    for(i = FDL_ONCE_MAX_WRITE_NUM; i > 0; i--)
    {
        Read_FLASH_MUIData(FDL_Block0 + (i - 1) * sizeof(Flash_Data), (uint8_t*)&Flash_Data, sizeof(Flash_Data));
        if(Flash_Data.Check == 0x5AA5)
        {
            if(i == FDL_ONCE_MAX_WRITE_NUM)//flash，满了需要擦除
            {
                FLASH_EraseOnePage(FDL_Block0);//block 0
                FDL_Block0_Save_Times = 0;
                Flash_WriteStruct(FDL_Block0, (uint8_t*)&Flash_Data, sizeof(Flash_Data));//并写入一次	
            }
            else
            {
                FDL_Block0_Save_Times = i - 1;//
		
            }
            break;
        }
    }
    
    /* 如果块0无数据或出错，初始化默认值 */
    if((i == 0))//内存块0无数据或出错
    {
        FLASH_EraseOnePage(FDL_Block0);//block 0
				Flash_Data.Check=0X5AA5;
				Flash_Data.Remote_Control_Num=0;
				Flash_Data.Max_Light_Value=100;
				Flash_Data.Light_CT_value=5700;//默认冷光
				Flash_Data.Electronic_Down_Limit=100;
				Flash_Data.Overweight_Correct=125;//
				Flash_Data.Electronic_Up_Limit=0;
				FDL_Block0_Save_Times=0;
				
				Flash_Data.Light_open_hour_time=23;
				Flash_Data.Light_open_min_time=0;
				Flash_Data.Light_close_hour_time=6;
				Flash_Data.Light_close_min_time=0;
				
				Flash_Data.Light_Bright_Enable=0;
				Flag.Brightness_time_flag=0;
				Flash_Data.Light_Bright_Value=20;
				Flash_Data.Beep_open=1;
				
				Flash_Data.Uptrend_Num_Correct=2;//出厂默认校正数是2 
				
    }	
    
    /* 读取块1的Flash数据 */
    for(i = FDL_BLOCK1_ONCE_MAX_WRITE_NUM; i > 0; i--)
    {
        Read_FLASH_MUIData(FDL_Block1 + (i - 1) * sizeof(Ell_Data), (uint8_t*)&Ell_Data, sizeof(Ell_Data));
        if((Ell_Data.Motor_Current_Position == 0xffff ) && (Ell_Data.Motor_Run_Circle == 0xffff ))
        {
            
        }
        else//有数据
        {
            if(i == FDL_BLOCK1_ONCE_MAX_WRITE_NUM)//flash，满了需要擦除
            {
                FLASH_EraseOnePage(FDL_Block1);//block 1
                FDL_Block1_Save_Times = 0;
                Flash_WriteStruct(FDL_Block1 + FDL_Block1_Save_Times * sizeof(Ell_Data), (uint8_t*)&Ell_Data, sizeof(Ell_Data)); 
            }
            else
            {
                FDL_Block1_Save_Times = i - 1;//
							
            }
            break;
        }
    }
 
    		      /* 如果块1无数据，初始化默认值 */
    if(i == 0)//内存块1无数据
    {
			FLASH_EraseOnePage(FDL_Block1);
			Ell_Data.Motor_Run_Circle = 0;	
			Ell_Data.B_Motor_Run_Circle = 0;	
	 
			Ell_Data.Motor_Current_Position = DOWN_POSITION_VALUE + 100;  //4100 ,3850
			Ell_Data.B_Motor_Current_Position = DOWN_POSITION_VALUE + 100;
	
    }
   motor_current_to_advalue(Ell_Data.Motor_Run_Circle);
	set_down_limit_value=Flash_Data.Electronic_Down_Limit*ONE_PERCENT_PULSE+UP_POSITION_VALUE;//电子下限位对应霍尔脉冲数
	if(Flash_Data.Electronic_Up_Limit==0)
	{
		set_up_limit_value=Flash_Data.Electronic_Up_Limit*ONE_PERCENT_PULSE+1000;//电子上限位对应霍尔脉冲数
	}
	else
	{
		set_up_limit_value=Flash_Data.Electronic_Up_Limit*ONE_PERCENT_PULSE+1000;
	}
	
	if((Flash_Data.Overweight_Correct==0xff)||(Flash_Data.Overweight_Correct==0))
	{
		Flash_Data.Overweight_Correct=125;	
	}
	
	if(Flash_Data.Max_Light_Value>100)
	{
		Flash_Data.Max_Light_Value=100;	
	}
	if(Flash_Data.Light_Bright_Value>100)
	{
		Flash_Data.Light_Bright_Value=100;	
	}
	if(Flash_Data.Light_CT_value>5700)
	{
		Flash_Data.Light_CT_value=5700;	
	}	
	if(Flash_Data.Uptrend_Num_Correct>(10-UPTREND_NUM))//碰撞上升趋势次数校正
	{
		Flash_Data.Uptrend_Num_Correct=10-UPTREND_NUM;//最高的校正次数
	}
 
	Device_State_Data.Beep_Enable=Flash_Data.Beep_open;
	Device_State_Data.electronic_down_limit=Flash_Data.Electronic_Down_Limit;
	Device_State_Data.electronic_up_limit=Flash_Data.Electronic_Up_Limit;
	
	Device_State_Data.Light_Bright=Flash_Data.Max_Light_Value;

	Device_State_Data.Light_colour_temperature=Flash_Data.Light_CT_value;
	colour_temperature_temp=(Device_State_Data.Light_colour_temperature-3000)/5;//步长5  0~540
	
	Device_State_Data.start_time_hour=Flash_Data.Light_open_hour_time;
	Device_State_Data.start_time_min=Flash_Data.Light_open_min_time;
	Device_State_Data.end_time_hour=Flash_Data.Light_close_hour_time;
	Device_State_Data.end_time_min=Flash_Data.Light_close_min_time;
	
	Device_State_Data.nightlight_switch=Flash_Data.Light_Bright_Enable;
	Device_State_Data.brightness=Flash_Data.Light_Bright_Value;
		
}
//存储块0
void save_data(void)
{	
	if(FDL_Block0_Save_Times>=FDL_ONCE_MAX_WRITE_NUM-2)//fdl区存满了，最后一块区留给掉电保存
	{
	
			FLASH_EraseOnePage(FDL_Block0);
		  FDL_Block0_Save_Times=0;
	}
	else
	{
		FDL_Block0_Save_Times++;	
	}
Flash_WriteStruct(FDL_Block0+FDL_Block0_Save_Times*sizeof(Flash_Data),(uint8_t*)&Flash_Data,sizeof(Flash_Data));//写入一次	
}
//存储块1
void save_current_position(void)
{	
	if(FDL_Block1_Save_Times>=FDL_BLOCK1_ONCE_MAX_WRITE_NUM-2)//fdl区存满了，最后一块区留给掉电保存
	{
	   FLASH_EraseOnePage(FDL_Block1);
		FDL_Block1_Save_Times=0;
	}
	else
	{
		FDL_Block1_Save_Times++;	
	}
  Flash_WriteStruct(FDL_Block1+FDL_Block1_Save_Times*sizeof(Ell_Data),(uint8_t*)&Ell_Data,sizeof(Ell_Data));//并写入一次
}
/*遥控器清码*/
void match_data_clear(void)
{
		Flash_Data.Remote_Control_Num=0;

		Flash_Data.Remote_Control_Id[0]= Capture_Command.Identify_data;//ID码
		Flash_Data.Remote_Control_Key[0]=Capture_Command.Secret_Key_data;//密钥
		Synchronous_Count_value[0]=0;//同步计数值

		Flash_Data.Remote_Control_Id[1]= Capture_Command.Identify_data;//ID码
		Flash_Data.Remote_Control_Key[1]=Capture_Command.Secret_Key_data;//密钥
		Synchronous_Count_value[1]=0;//同步计数值

		Flash_Data.Remote_Control_Id[2]= Capture_Command.Identify_data;//ID码
		Flash_Data.Remote_Control_Key[2]=Capture_Command.Secret_Key_data;//密钥
		Synchronous_Count_value[2]=0;//同步计数值
		Device_State_Data.Control_num=Flash_Data.Remote_Control_Num;
		Flash_WriteStruct(FDL_Block0_Save_Times*sizeof(Flash_Data),(uint8_t*)&Flash_Data,sizeof(Flash_Data));//写入一次	
}

/*遥控器对码及添码处理*/
void match_data_del(void)
{
	uint8_t i;	
	for(i=0;i<Flash_Data.Remote_Control_Num;i++)
	{
		if((Capture_Command.Identify_data==Flash_Data.Remote_Control_Id[i])//已经存在遥控器
		&&(Capture_Command.Secret_Key_data==Flash_Data.Remote_Control_Key[i]))
		{
			return;
		}	
	}
if(FDL_Block0_Save_Times>=FDL_ONCE_MAX_WRITE_NUM-2)//fdl区存满了
	{		
	while(FLASH_EraseOnePage(FDL_Block0) != 0)
       FDL_Block0_Save_Times=0;
	}
	else
	{
	FDL_Block0_Save_Times++;
  }
	if(Flash_Data.Remote_Control_Num>=3)
	{
		for(i=0;i<2;i++)
		{
			Flash_Data.Remote_Control_Id[i]=Flash_Data.Remote_Control_Id[i+1];//ID
			Flash_Data.Remote_Control_Key[i]=Flash_Data.Remote_Control_Key[i+1];//密钥
			Synchronous_Count_value[i]=Synchronous_Count_value[i+1];//同步计数值
		}
		Flash_Data.Remote_Control_Id[2]= Capture_Command.Identify_data;//ID码
		Flash_Data.Remote_Control_Key[2]=Capture_Command.Secret_Key_data;//密钥
		Synchronous_Count_value[2]=0;//同步计数值
	}
	else 
	{			
		Flash_Data.Remote_Control_Id[Flash_Data.Remote_Control_Num]= Capture_Command.Identify_data;//ID码
		Flash_Data.Remote_Control_Key[Flash_Data.Remote_Control_Num]=Capture_Command.Secret_Key_data;//密钥
		Synchronous_Count_value[Flash_Data.Remote_Control_Num]=0;
		Flash_Data.Remote_Control_Num++;	
	}		
	Flash_WriteStruct(FDL_Block0+FDL_Block0_Save_Times*sizeof(Flash_Data),(uint8_t*)&Flash_Data,sizeof(Flash_Data));//写入一次		
  Device_State_Data.Control_num=Flash_Data.Remote_Control_Num;
}
//掉电保存
void _220AC_Power_down_save(void)//掉电保存  
{
	if(!Flag.Already_save_data_flag)//未进行掉电保存
	{
		if(Flag._220AC_Power_Down_flag)
		{
		  save_data();
			 save_current_position();
		  MOTOR_CLOSE_IO;  
			Flag.Already_save_data_flag=1;//已掉电保存一次
			Flag._220AC_Power_Down_flag=1;
		}
	}
}
/* 变量初始化 */
void variable_init(void)
{	
    BuzVcc_OFF;
    current_state=0;
    Holl_Circle_num=0;
    Motor_Circle_num=0;
    Capture_Command.Capture_Pulse_Wide_Sequence=0;
    Function_select.AD_sum=0;   
    memset((uint8_t *)&Time,0,sizeof(Time));
    Time.Light_key_space_time=Light_KEY_SPACE_TIME;//防止上电第一次开灯，带切换色温功能
    memset((uint8_t *)&Synchronous_Count_value,0,sizeof(Synchronous_Count_value));
    memset((uint8_t *)&Led,0,sizeof(Led));
    memset((uint8_t *)&Up_State_Data,0xff,sizeof(Up_State_Data));//   
    Device_State_Data.current_speed=35;//上电上报一下默认速度3.5%/1S;
    Led.Led_Orange_state=LED_ORG_STATE_ON;//橙灯上电常亮状态   
    Net_state=NET_OFFLINE;  
	  Device_State_Data.Control_num=Flash_Data.Remote_Control_Num;
}






