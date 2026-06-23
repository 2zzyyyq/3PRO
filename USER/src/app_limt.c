 #include "ALL.h"
 // 电机控制相关变量
  uint8_t conform_motor_obs_num, conform_motor_obs_num2;
  uint8_t A_obs_correction;
  uint16_t motor_current_to_advalue(float  circle);
//  void Ovreweight_current_correct(void); // 超重按键校准(两人均匀分布负载，每人20kg)
 uint8_t DEBUG ;
/*电机信号检测及报警处理*/
void alarm_signal_test_and_beep(void)
{
    uint8_t  i;
    static uint8_t _10ms_step=0;
    static uint16_t sum=0;               // 10次值取平均
    if((Device_State_Data.Motor_State_Byte==MOTOR_UP_STATE)||(Device_State_Data.Motor_State_Byte==MOTOR_DOWN_STATE))// 电机运转中
    {
			if(YZ)
			{
				Time.Motor_Stop_Encounter_Obstacle_time++;
				if(Time.Motor_Stop_Encounter_Obstacle_time>=ENCOUNTER_Obstacle_TIME1)// 遇阻停止
				{
					Flag.Yz=1;
					Time.Motor_Stop_Encounter_Obstacle_time=0;
					if(!Err.err_bit.Motor_Obs)// 非碰撞状态
					{
						Device_State_Data.Motor_State_Byte=MOTOR_SUSPEND_STATE;
						SetupBzhx(LONG_BEEP,Flash_Data.Beep_open);// 蜂鸣一声
						Flag.Encounter_Obstacle_Motor_stop=1;// 遇阻停止

						Up_State_Data.Motor_Position=0xff;
						Flag.Run_to_set_position_flag=0;// 运行到指定位置过程标志清零
					}
				}
			}
			else
			{
				if(Time.Motor_No_Encounter_Obstacle_time>LIMIT_SIGNAL_FITER_TIME)
				{
					Flag.Yz=0;
					Time.Motor_No_Encounter_Obstacle_time=0;
					if(Err.err_bit.Encounter_Obstacle)
					{
						Err.err_bit.Encounter_Obstacle=0;
						Time.Encounter_Obstacle_beepnum=0;
					}
				}
				Time.Motor_Stop_Encounter_Obstacle_time=0;
			}
    }
    else// 电机停止
    {
			if(YZ)
			{
				if(!Err.err_bit.Encounter_Obstacle)
				{
					if(Time.Motor_Stop_Encounter_Obstacle_time>=ENCOUNTER_Obstacle_TIME2)// 遇阻信号超过3s报错
					{
						Flag.beep_off_5stime_flag=1;
						Time.beep_off_5stime=0;
						Time.Encounter_Obstacle_beepnum=0;
						Err.err_bit.Encounter_Obstacle=1;
					}
					else
					{
						Time.Motor_Stop_Encounter_Obstacle_time++;
						if(Time.Motor_Stop_Encounter_Obstacle_time>=ENCOUNTER_Obstacle_TIME1)
						{
							Flag.Yz=1;
						}
					}
				}
				Time.Motor_No_Encounter_Obstacle_time=0;
		}
		else
		{
			if(Time.Motor_No_Encounter_Obstacle_time>LIMIT_SIGNAL_FITER_TIME)// 确认不是误触发
			{
				Flag.Yz=0;
				Time.Motor_No_Encounter_Obstacle_time=0;
				if(Err.err_bit.Encounter_Obstacle)
				{
					Err.err_bit.Encounter_Obstacle=0;
					Time.Encounter_Obstacle_beepnum=0;
					SetupBzhx(BEEP_PWM_OFF,1);// 取消遇障报警
				}
				else
				{
					if(Flag.Encounter_Obstacle_Motor_stop==1)// 遇阻停止后的无障碍物自动抬升
					{
						Flag.Encounter_Obstacle_Motor_stop=0;
					}
					else
					{
						if(ENCOUNTER_Obstacle_TIME3<Time.Motor_Stop_Encounter_Obstacle_time)// 小于3s大于200ms的抬升
						{
							if((!Flag.Up_limit)&&(!(Ell_Data.Motor_Current_Position<set_up_limit_value)))// 如果没有上限位和电子上限位
							{
								Device_State_Data.Motor_State_Byte=MOTOR_UP_STATE;// 电机上升
								Flag.Motor_raise_up=1;
								Time.Motor_Run_time=0;
								Holl_pulse_temp=Ell_Data.Motor_Current_Position;
							}
						}
					}
				}
				Time.Motor_Stop_Encounter_Obstacle_time=0;
			}
		}
    }
    switch(current_state)// 电机堵转检测状态机
    {
	    case 0:
	    {
	    	if(Motor_Current.ADvalue>MOTOR_ERR_CURRENT_ADVALUE)// 大于堵转电流
			{
				Time.Judge_Motor_Err_time++;
				if(Time.Judge_Motor_Err_time>MOTOR_STALL_TIME)// 100ms
				{
					Time.Judge_Motor_Err_time=0;
					Flag.beep_off_5stime_flag=1;
					Time.beep_off_5stime=0;
					Time.Motor_Err_beepnum=0;
					Time.Up_Data_Delay_time=1000;// 延时1s上报故障
					Err.err_bit.Motor_Err=1;
					motor_stop();// 电机停止
					Device_State_Data.Motor_State_Byte=MOTOR_SUSPEND_STATE;// 停止
					Flag.No_judge_electronic_limit=0;
				}
			}
			else
			{
				if(Motor_Current.ADvalue<(MOTOR_ERR_CURRENT_ADVALUE-MOTOR_ERR_CURRENT_ADVALUE_OFFSET))// 小于堵转判断恢复值
				{
					Time.Judge_Motor_Err_time=0;
				}
				if(Flag.Motor_run_now)
				{
					if(Time.Motor_Run_time<200)// 继电器上电后延时2s
					{
						Time.Motor_Run_time++;
						if(Time.Motor_Run_time==95)
						{
							Time.Test_speed_time=0;

							Holl_test_speed_temp=Ell_Data.Motor_Current_Position;
						}
						else if(Time.Motor_Run_time==195)// 得到速度值(单位: 1%/100ms行程)
						{
							if(Ell_Data.Motor_Current_Position>Holl_test_speed_temp)
							{
								Device_State_Data.current_speed=(Ell_Data.Motor_Current_Position-Holl_test_speed_temp)/2.8;
							}
							else if(Ell_Data.Motor_Current_Position<Holl_test_speed_temp)
							{
								Device_State_Data.current_speed=(Holl_test_speed_temp-Ell_Data.Motor_Current_Position)/2.8;
							}
						}
						Motor_Current_Position_pulse=10000;
						Holl_pulse_temp=10000;
						Time.Motor_Err_time=0;
					}
					else
					{
						if(Time.Motor_Err_time++>100)// 1s检测一次位置变化
						{
							Time.Motor_Err_time=0;
							if(((Device_State_Data.Motor_State_Byte==MOTOR_UP_STATE)&&(Motor_Current_Position_pulse>=Holl_pulse_temp))// 上升时位置不在变大则堵转
							    ||((Device_State_Data.Motor_State_Byte==MOTOR_DOWN_STATE)&&(Motor_Current_Position_pulse<=Holl_pulse_temp)))// 下降时位置不在变小则堵转
							{
								Flag.beep_off_5stime_flag=1;
								Time.beep_off_5stime=0;
								Time.Motor_Err_beepnum=0;
								Err.err_bit.Motor_Err=1;
								Time.Up_Data_Delay_time=1000;// 延时1s上报故障，防止app在更新电机状态时收到故障信息
								motor_stop();// 电机停止
								Flag.No_judge_electronic_limit=0;
								Device_State_Data.Motor_State_Byte=MOTOR_SUSPEND_STATE;// 暂停
							}
							else
							{
								Holl_pulse_temp=Motor_Current_Position_pulse;
								Err.err_bit.Motor_Err=0;
							}
						}
					}
				}
				else
				{
					Device_State_Data.current_speed=40;
				}
				if((Flag.Enable_Test_Over_Wight_flag)&&(!Flag.Overweight_current_correct_start))// 使能检测超重
				{
					if(Motor_Current.ADvalue>motor_current_to_advalue(Ell_Data.Motor_Run_Circle))// 大于超重电流
				        {
				    		Time.Judge_Over_Wight_time++;
						if(Time.Judge_Over_Wight_time>=MOTOR_OVER_WEIGHT_TIEM)// 确认超重电流
						{
							Time.Judge_Over_Wight_time=0;
							current_state=2;
						}
				        }
					else
					{
						if(Motor_Current.ADvalue<(motor_current_to_advalue(Ell_Data.Motor_Run_Circle)
						-OVER_WIGHT_CURRENT_ADVALUE_OFFSET))// 小于超重电流判断恢复阈值
						{
							Time.Judge_Over_Wight_time=0;
							Err.err_bit.Over_Wight=0;
						}
					}
				}
				if((Flag.Enable_Test_Motor_Obs_flag)&&(Device_State_Data.Motor_Position>3))// 使能检测电机上行障碍物
				{
					sum += Motor_Current.ADvalue;
					_10ms_step++;
					if(_10ms_step>=AD_AVERAGE_NUM)// 100ms
					{
						_10ms_step=0;
						if(ADC_sample_num<AD_BUFF_DIP)
						{
							ADC_sample_num++;
						}
						ADC_read_buf[ADC_sample_num-1]=sum/AD_AVERAGE_NUM;
						sum=0;
						if(ADC_sample_num==AD_BUFF_DIP)
						{
							ADC_sample_max=ADC_read_buf[0];
							conform_motor_obs_num=0;
							for(i=1;i<AD_BUFF_DIP;i++)
							{
								if(ADC_read_buf[i]>ADC_sample_max)
								{
									ADC_sample_max=ADC_read_buf[i];
									conform_motor_obs_num++;
								}
								if(i<(AD_BUFF_DIP-1))
								{
									if((ADC_read_buf[i]<ADC_read_buf[i-1])&&(ADC_read_buf[i+1]<ADC_read_buf[i]))
									{
										conform_motor_obs_num=0;
									}
								}
							}
							for(i=0;i<AD_BUFF_DIP-1;i++)
							{
								ADC_read_buf[i]=ADC_read_buf[i+1];
							}
							if(conform_motor_obs_num>=(UPTREND_NUM+Flash_Data.Uptrend_Num_Correct))// 判断为电机上行障碍
							{
								current_state=1;
							}
						}
					}
				}
				else
				{
					_10ms_step=0;
				}
			}
	    }break;
	    case 1:       // 上行遇障处理
	    {
		    Flag.Run_to_set_position_flag=0;
		    Err.err_bit.Motor_Obs=1;
		    Flag.beep_off_5stime_flag=1;
		    Time.beep_off_5stime=0;
		    Time.Encounter_Obstacle_beepnum=0;
		    Time.Motor_Obs_Down_time=0;
			Device_State_Data.Motor_State_Byte=MOTOR_DOWN_STATE;// 下降
			Up_State_Data.Err_Byte&=0xf7;// 清除上报告警位
			Flag.Motor_up_to_down=1;
			Time.Motor_Dir_Change_Delaytime=500;// 延时500ms换向
			current_state=4;
	    }break;
	    case 2:       // 超重处理
	    {
		    Flag.beep_off_5stime_flag=1;
		    Time.beep_off_5stime=0;
		    Time.Over_Wight_beepnum=0;
		    Err.err_bit.Over_Wight=1;
		    Up_State_Data.Err_Byte&=0xfd;// 清除超重上报告警位
		    Device_State_Data.Motor_State_Byte=MOTOR_DOWN_STATE;// 下降
		    Flag.Motor_up_to_down=1;
		    Time.Motor_Dir_Change_Delaytime=500;// 换向延时500ms
		    current_state=3;
	    }break;
	    case 3:       // 超重后等待
	    {
	    	if((Device_State_Data.Motor_State_Byte==MOTOR_DOWN_STOP_STATE)
	    	||(Device_State_Data.Motor_State_Byte==MOTOR_SUSPEND_STATE))// 超重后需要到下限位才停止
			{
			    current_state=0;
			}
	    }break;
	    case 4:       // 遇阻下降后等待
	    {
			Time.Motor_Stop_Encounter_Obstacle_time=0;// 遇障碍物判断，计时清零
	    	if(Time.Motor_Obs_Down_time++>300)
			{
				Device_State_Data.Motor_State_Byte=MOTOR_SUSPEND_STATE;
				Time.Motor_Obs_Down_time=0;
			}
			if((Device_State_Data.Motor_State_Byte==MOTOR_DOWN_STOP_STATE)
		    	||(Device_State_Data.Motor_State_Byte==MOTOR_SUSPEND_STATE))// 暂停
			{
			    current_state=0;
			    Err.err_bit.Motor_Obs=0;
			}
	    }break;
	    case 5:       // 强制下降
	    {
	    	Time.Motor_Stop_Encounter_Obstacle_time=0;// 遇障碍物判断，计时清零
			if(Time.Motor_Force_Down_time++>100)
			{
				if((Flag.Yz)||(Device_State_Data.Motor_Position==100))
				{
					Device_State_Data.Motor_State_Byte=MOTOR_SUSPEND_STATE;
				}
				Time.Motor_Force_Down_time=0;
				current_state=0;
				Flag.Motor_force_down=0;
			}
			if((Device_State_Data.Motor_State_Byte==MOTOR_SUSPEND_STATE)||(Device_State_Data.Motor_State_Byte==MOTOR_UP_STATE))
			{
				Time.Motor_Force_Down_time=0;
				current_state=0;
				Flag.Motor_force_down=0;
			}
	    }break;
	    default:break;
	}
    if(Flag.beep_off_5stime_flag)// 报警提示音循环时间到
    {
      	Flag.beep_off_5stime_flag=0;
		if((Err.err_bit.Encounter_Obstacle)||(Err.err_bit.Motor_Obs))
		{
            if((++Time.Encounter_Obstacle_beepnum)<=ALARM_YZ_BEEP_NUM)
		    {
			   SetupBzhx(YZ_ALARM_BEEP,1);
		    }
		    else
		    {
		    	Time.Encounter_Obstacle_beepnum=ALARM_YZ_BEEP_NUM;
		    }
		}
		else if(Err.err_bit.Motor_Err)
		{
		    if((++Time.Motor_Err_beepnum)<=ALARM_MOTOR_BEEP_NUM)
		    {
		 	SetupBzhx(MOTOR_ERR_ALARM_BEEP,1);
		    }
		    else
		    {
		    	Time.Motor_Err_beepnum=ALARM_MOTOR_BEEP_NUM;
		    }
		}
		else if(Err.err_bit.Over_Wight)
		{
            if((++Time.Over_Wight_beepnum)<=ALARM_OVER_WEIGHT_BEEP_NUM)
		    {

		    	SetupBzhx(OVER_WIGHT_ALARM_BEEP,1);
		     }
		    else
		    {
		    	Time.Over_Wight_beepnum=ALARM_OVER_WEIGHT_BEEP_NUM;
		    }
		}
     }
     Device_State_Data.Err_Byte=Err.data;
}
/*限位检测保护*/
void limit_protect(void)
{
	if(!UP_LIMIT)
	{
		Time.return_to_up_time=0;
		Flag.Up_limit=0;
	}
	if(!DOWN_LIMIT)
	{
		Time.return_to_down_time=0;
		Flag.Down_limit=0;
	}
	if(!Flag.Motor_force_down)
	{
		if(UP_LIMIT)// 到达机械上限位
		{
			if(Time.return_to_up_time>=LIMIT_SIGNAL_FITER_TIME)
			{
				Flag.Up_limit=1;
				Device_State_Data.Motor_Position=0;
				Ell_Data.Motor_Current_Position=UP_POSITION_VALUE;
				Time.return_to_up_time=0;
				Flag.Overweight_current_correct_start=0;// 停止校准
				if(Device_State_Data.Motor_State_Byte==MOTOR_UP_STATE)
				{
					Device_State_Data.Motor_State_Byte=MOTOR_UP_STOP_STATE;
					SetupBzhx(LONG_BEEP,Flash_Data.Beep_open);
					Motor_Circle_num++;
					if(Motor_Circle_num==10)
					{
						Motor_Circle_num=0;
						Ell_Data.Motor_Run_Circle++;
						if(Ell_Data.Motor_Run_Circle>=MOTOR_MAX_CIRCLE)
						{
							Ell_Data.Motor_Run_Circle=MOTOR_MAX_CIRCLE;
						}
					}
					#if DEBUG// 测试程序自动下降
						Device_State_Data.Motor_State_Byte=MOTOR_DOWN_STATE;
						Flag.Motor_up_to_down=1;
					#endif
				}
			}
		}
		else
		{
			if(Flag.Run_to_set_position_flag==1)// 收到了指定位置命令
			{
				if(Device_State_Data.Motor_State_Byte==MOTOR_DOWN_STATE)
				{
					if(Ell_Data.Motor_Current_Position>=set_position_pulse)
					{
						Flag.Run_to_set_position_flag=0;
						if(set_position_pulse<set_down_limit_value)
						{
							Device_State_Data.Motor_State_Byte=MOTOR_SUSPEND_STATE;
						}
						else
						{
							Device_State_Data.Motor_State_Byte=MOTOR_DOWN_STOP_STATE;
						}
						SetupBzhx(LONG_BEEP,Flash_Data.Beep_open);
						Err.err_bit.Motor_Obs=0;
					}
				}
				else if(Device_State_Data.Motor_State_Byte==MOTOR_UP_STATE)
				{
					if(Ell_Data.Motor_Current_Position<=set_position_pulse)
					{
						Flag.Run_to_set_position_flag=0;
						if(set_position_pulse>set_up_limit_value)
						{
							Device_State_Data.Motor_State_Byte=MOTOR_SUSPEND_STATE;
						}
						else
						{
							Device_State_Data.Motor_State_Byte=MOTOR_UP_STOP_STATE;
						}
						SetupBzhx(LONG_BEEP,Flash_Data.Beep_open);
					}
				}
			}
			else
			{
				if(Ell_Data.Motor_Current_Position>(set_down_limit_value-20))
				{
					if((Ell_Data.Motor_Current_Position<DOWN_POSITION_VALUE)&&(Flag.No_judge_electronic_limit))
					{
						// 电子限位设置到下限位置
					}
					else
					{
						if(Device_State_Data.Motor_State_Byte==MOTOR_DOWN_STATE)
						{
							Device_State_Data.Motor_State_Byte=MOTOR_DOWN_STOP_STATE;
							SetupBzhx(LONG_BEEP,Flash_Data.Beep_open);
						}
					}
				}
				else if((Ell_Data.Motor_Current_Position<=(set_up_limit_value+20))&&(!Flag.No_judge_electronic_limit))// 到达电子上限位
				{
					if(Device_State_Data.Motor_State_Byte==MOTOR_UP_STATE)
					{
						Flag.Overweight_current_correct_start=0;// 停止校准
						Device_State_Data.Motor_State_Byte=MOTOR_UP_STOP_STATE;
						SetupBzhx(LONG_BEEP,Flash_Data.Beep_open);
						Motor_Circle_num++;
						if(Motor_Circle_num==10)
						{
							Motor_Circle_num=0;
							Ell_Data.Motor_Run_Circle++;
							if(Ell_Data.Motor_Run_Circle>=MOTOR_MAX_CIRCLE)
							{
								Ell_Data.Motor_Run_Circle=MOTOR_MAX_CIRCLE;
							}
						}
					}
				}
				else
				{

				}
			}
		}
	}
}