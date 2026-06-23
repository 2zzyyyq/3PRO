#include "main.h"
#include "ALL.h"

void HAL_TIM_CNT_IRQHandler_BACK(void);
void HAL_TIM_CNT_IRQHandler_BACK(void)
{
 
    /* Start user code. Do not edit comment generated here */
    Time.time5ms++;
    Time.time10ms++;
    Time.time100ms++;
    Time.time150ms++;
    Time.Test_speed_time++;
    Time.return_to_up_time++;
    Time.return_to_down_time++;
    Time.Motor_No_Encounter_Obstacle_time++;
    Time.Enter_Pcba_test_time++;
    
    if(Device_State_Data.Light_State_Byte==LIGHT_OFF)
    {
	 if(Time.Light_key_space_time<Light_KEY_SPACE_TIME)
	 {
   		 Time.Light_key_space_time++;
	 }
    }
    else
    {
    	Time.Light_key_space_time=0;
    }
    
			if(STOP_CMD_Repeat!=0)
			{
			Time.Del_Match_Code_time++;
			if(Time.Del_Match_Code_time>5000)
			{
			Time.Del_Match_Code_time=0;
			STOP_CMD_Repeat=0;
			}
    } 
    else
    {
    	Time.Del_Match_Code_time=0;
    }
    if(Time.Holl_Pluse_Filter_time<0xff)
    {
   	 Time.Holl_Pluse_Filter_time++;
    }
    if(Time.Uart_Receive_Interval_time<RECEIVE_DATA_PACKET_INTERVAL_TIME)
    {
   	 Time.Uart_Receive_Interval_time++;
    }
    if(Time.Uart2_Receive_Interval_time<RECEIVE_DATA_PACKET_INTERVAL_TIME)
    {
   	 Time.Uart2_Receive_Interval_time++;
    }
    if(Time.time5ms>=5)
    {
	    Time.time5ms=0;
    	 Flag.time5ms_flag=1;
    }
    if(Time.time10ms>=10)
    {
	    Time.time10ms=0;
    	 Flag.time10ms_flag=1;
    }
    if(Time.time100ms>=100)
    {
	      Time.time100ms=0;
    	 Flag.time100ms_flag=1;
	 if(Flag.Overweight_current_correct_start)
	 {
	 	Time.Overweight_Current_time++;
	 }
	 else
	 {
	 	Time.Overweight_Current_time=0;
	 }
    }   
    if(Time.time150ms>=150)
    {
	        Time.time150ms=0;
    	    Flag.time150ms_flag=1;
    } 
    if(Time.Up_Data_Delay_time>0)
    {
    	Time.Up_Data_Delay_time--;
			if(Time.Up_Data_Delay_time==0)
			{
				Up_State_Data.Err_Byte&=0xfb;//重新上报电机故障
			}
    }
		
			// 允许添码时间处理
	if (Device_State_Data.Control_open == 1)
	{
			Time.Enable_Add_Match_Code_time++;
			if (Time.Enable_Add_Match_Code_time >= EN_ADD_CODE_TIME) // 添码10s有效
			{

				//  Flag.Add_Match_Code_flag = 0;
					Device_State_Data.Control_open=0;
					Flash_Data.Control_open=Device_State_Data.Control_open;
			}
	}
	else
	{
			Time.Enable_Add_Match_Code_time = 0;
	}
    if(!Flag._220AC_Power_Down_flag)//
    {
    	Time.Zero_crossing_no_single_time++;
				if(Time.Zero_crossing_no_single_time>ZERO_CROSSING_NO_SINGL_TIME)
				{
					Flag._220AC_Power_Down_flag=1;//掉电
					MOTOR_DN_IO(0);//停止电机
					MOTOR_UP_IO(0);
				}
    }
    if(Flag.Motor_Start_up)//电机开始上升
    {
    	Time.Motor_Start_Up_time++;
			if(Time.Motor_Start_Up_time>=DIS_JUDGE_MOTOR_UP_OBS_TIME)//启动后延时一段时间开始检测上升遇到障碍物
			{
				Flag.Enable_Test_Motor_Obs_flag=1;
				ADC_sample_num=0;
				ADC_sample_max=0;
				conform_motor_obs_num=0;
				ADC_sample_min=65535;
			}
			 if(Time.Motor_Start_Up_time>=DIS_JUDGE_MOTOR_OVER_WEIGHT_TIME)//启动后延时一段时间开始检测超重
			{
				Flag.Motor_Start_up=0;
				Time.Motor_Start_Up_time=0;
				Flag.Enable_Test_Over_Wight_flag=1;
				Time.Judge_Over_Wight_time=0;
			}
    }
    if(Flag.Motor_down_to_up)//电机换向中
    {
				Time.Motor_Dir_Change_Delaytime++;
    	if(Time.Motor_Dir_Change_Delaytime>=MOTOR_DIR_CHANGE_DELAY)
			{
				Time.Motor_Dir_Change_Delaytime=0;
				Flag.Motor_down_to_up=0;
				SetupBzhx(MOTOR_UP_BEEP,Flash_Data.Beep_open);//延时完响应蜂鸣器
			}
    }
    else if(Flag.Motor_up_to_down)//电机换向中
    {	
    	Time.Motor_Dir_Change_Delaytime++;
    	if(Time.Motor_Dir_Change_Delaytime>=MOTOR_DIR_CHANGE_DELAY)
			{
				Time.Motor_Dir_Change_Delaytime=0;
				Flag.Motor_up_to_down=0;
				if(current_state==3)
				{
					Up_State_Data.Err_Byte&=0xfd;//重新上报超重
				}
				SetupBzhx(MOTOR_DOWN_BEEP,Flash_Data.Beep_open);//切换完再响蜂鸣器
			}
    }
    else if(Flag.Motor_raise_up)//轻抬上升
    {	
    	Time.Motor_Dir_Change_Delaytime++;
    	if(Time.Motor_Dir_Change_Delaytime>=MOTOR_DIR_CHANGE_DELAY1)
			{
				Time.Motor_Dir_Change_Delaytime=0;
				Flag.Motor_raise_up=0;
				SetupBzhx(GENTLY_LIFT_MOTOR_UP_BEEP,Flash_Data.Beep_open);//切换完再响蜂鸣器	
			}
    }
    if(Err.data!=0)//有报警信号
    {
			 Time.beep_off_5stime++;
			 if(Time.beep_off_5stime>=ALARM_BEEP_CYCLE_TIME)
			 { 
					Time.beep_off_5stime=0;
					Flag.beep_off_5stime_flag=1;	
			 }
    }
    if(Flag.power_on_10stime_flag==0)
    {
    	Time.power_on_10stime++;
   	if(Time.power_on_10stime>=POWER_ON_ENABLE_STUDY_TIME)
        {
					Time.power_on_10stime=0;
				  Flag.power_on_10stime_flag=1;
    	} 
    }
    /* End user code. Do not edit comment generated here */
}