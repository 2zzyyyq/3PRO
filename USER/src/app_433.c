#include "ALL.h" 
void App_Timer1PortCfg(void);
void App_Timer1Cfg(void);
void RF_DEC(unsigned char *source);
void Keelop_KEY(unsigned long  key_write);
void capture_433key_judge_answer(button* handle,uint8_t power_on_10stime_flag);

uint8_t analysis_capture_command(CAPTURE_COMMAND *Capture_Command,uint8_t power_on_10stime_flag);
volatile FDL_DATA  Flash_Data;
CAPTURE_COMMAND  Capture_Command;
button key_set_key,capture_433key;
uint8_t Study_Cmd_Repeat,STOP_CMD_Repeat;//连续按两下添码
FLAG Flag;

//不同功能支持的遥控器命
const uint8_t Function_Cmd[7][11]=
{
	{0x11,0x33,0x55,0x0f,0x79,0x88,0x99,0xAA,0xBB,0x00,0xFE},//照明功能
	{0x11,0x33,0x55,0x0f,0x79,0x73,0x00,0x00,0x00,0x00,0xFE},//照明消毒
	{0x11,0x33,0x55,0x0f,0x79,0x75,0x00,0x00,0x00,0x00,0xFE},//照明风干
	{0x11,0x33,0x55,0x0f,0x79,0x76,0x75,0x00,0x00,0x00,0xFE},//照明烘干风干
	{0x11,0x33,0x55,0x0f,0x79,0x73,0x75,0x00,0x00,0x00,0xFE},//照明消毒风干
	{0x11,0x33,0x55,0x0f,0x79,0x73,0x75,0x76,0x00,0x00,0xFE},//照明消毒风干烘干
};


/*判断遥控器是否有效*/
uint8_t judge_remote_control(uint32_t roll_code)
{

	uint8_t i=0;
	uint8_t  data[4];
	//uint16_t roll_code_count;		
	if(Flash_Data.Remote_Control_Num==0)//还没有遥控器数据
	{
		return false;
	}
	else
	{	if((Capture_Command.Identify_data==0x00000000)&&(Capture_Command.Secret_Key_data==0x5AA55AA5))//预埋遥控器
		{
			return true;
		}
		for(i=0;i<Flash_Data.Remote_Control_Num;i++)//查找是否有存储的遥控器ID
		{
			if(Capture_Command.Identify_data==Flash_Data.Remote_Control_Id[i])
			
			{	
				data[0]=(uint8_t)(roll_code>>24);
				data[1]=(uint8_t)(roll_code>>16);
 				data[2]=(uint8_t)(roll_code>>8);
  		  data[3]=(uint8_t)(roll_code);			
			  Keelop_KEY(Flash_Data.Remote_Control_Key[i]);	
	      RF_DEC((uint8_t*)(&data));
			                                                                                       	//roll_code_count=data[0]*256+data[1];
				if((data[3]==Capture_Command.Command_data)&&(data[2]==((Capture_Command.Identify_data>>8)&0xff)))//对应的秘钥正确	
				{
					/*
					if(roll_code_count>(Synchronous_Count_value[i]+100))
					{		
						if(Synchronous_Count_value[i]==0)
						{
							Synchronous_Count_value[i]=roll_code_count;
							return true;
						}
						return false;//同步计数值不符合
					}			
					else
					{	
						Synchronous_Count_value[i]=roll_code_count;
						return true;
					}*/
					return true;
				}
			}
		}
		return false;
	}


}
void Record_PulseWidth(uint32_t g_tau0_ch7_width,CAPTURE_COMMAND *Capture_Command)
{
	
 
	static uint16_t real_tau0_ch7_width=0;
	if((HEAD_HIGH_MIN_TIME<=g_tau0_ch7_width)&&(g_tau0_ch7_width<=HEAD_HIGH_MAX_TIME))
	{
	    real_tau0_ch7_width=0;
	    Capture_Command->Capture_Pulse_Wide_value[0]=g_tau0_ch7_width;	 
	    Capture_Command->Capture_Pulse_Wide_Sequence=1;	    
	    Capture_Command->Capture_State=CAPTURE_FIXED_CODE;
	    return;
	}
	switch(Capture_Command->Capture_State)
	{
	   case CAPTURE_NO_DATA:
	   {
	    	Capture_Command->Capture_State=CAPTURE_HEAD_HIGH;//有遥控器信号
		    Capture_Command->Capture_Pulse_Wide_Sequence=0;
	   }break;
	  case CAPTURE_FIXED_CODE://固定码和引导高电平
	   {		  
	   	if(!_433_DATA)//本次读取的是高电平
		{
			if(g_tau0_ch7_width<INTERFERE_PLUSE_WIDTH)//认为是低电平受到干扰
			{
				Flag.Need_433_data_compensate=1;
				real_tau0_ch7_width+=g_tau0_ch7_width;
				if(Capture_Command->Capture_Pulse_Wide_Sequence>=2)
				{
					Capture_Command->Capture_Pulse_Wide_Sequence--;
				}
			}
			else
			{
				real_tau0_ch7_width=g_tau0_ch7_width;
				Flag.Need_433_data_compensate=0;
			}
		}
		else//本次读取低电平
		{
			if(Flag.Need_433_data_compensate)//上一个电平是干扰，
			{
				real_tau0_ch7_width+=g_tau0_ch7_width;	
				if(Capture_Command->Capture_Pulse_Wide_Sequence>=2)
				{
					Capture_Command->Capture_Pulse_Wide_Sequence--;
				}
			}
			else
			{
				real_tau0_ch7_width=g_tau0_ch7_width;
			}
			Flag.Need_433_data_compensate=0;
		}		
		if((real_tau0_ch7_width>_800US_MAX_TIME)&&(Capture_Command->Capture_Pulse_Wide_Sequence>1))//判定出错
		{
			               Capture_Command->Capture_State=CAPTURE_HEAD_HIGH;
	                   Capture_Command->Capture_Pulse_Wide_Sequence=0;	
			               Capture_Command->Capture_State=CAPTURE_NO_DATA;
			
			               return;
		}
		Capture_Command->Capture_Pulse_Wide_value[Capture_Command->Capture_Pulse_Wide_Sequence]=real_tau0_ch7_width;
		Capture_Command->Capture_Pulse_Wide_Sequence++;
		if(Capture_Command->Capture_Pulse_Wide_Sequence==FIXED_CODE_LEVELS+2)//捕获66个电平时间
		{
		     Capture_Command->Capture_State=CAPTURE_ROLL_CODE;
		     Capture_Command->Capture_433key_Judgetime=0;
		}
	   }break;
	   case CAPTURE_HEAD_HIGH://引导高电平
	   {
	  Capture_Command->Capture_Pulse_Wide_value[0]=g_tau0_ch7_width;
		if((HEAD_HIGH_MIN_TIME<=Capture_Command->Capture_Pulse_Wide_value[0])&&(Capture_Command->Capture_Pulse_Wide_value[0]<=HEAD_HIGH_MAX_TIME))
		{ 	
		    Capture_Command->Capture_Pulse_Wide_Sequence++;
		    Capture_Command->Capture_State=CAPTURE_FIXED_CODE;
		}
		
	   }break;
   
	  /* case CAPTURE_HEAD_LOW://引导低电平
	   {
	   	Capture_Command->Capture_Pulse_Wide_value[1]=g_tau0_ch7_width;
		if((HEAD_LOW_MIN_TIME<=Capture_Command->Capture_Pulse_Wide_value[1])&&(Capture_Command->Capture_Pulse_Wide_value[1]<=HEAD_LOW_MAX_TIME))
		{
		    Capture_Command->Capture_Pulse_Wide_Sequence++;
		    Capture_Command->Capture_State=CAPTURE_FIXED_CODE;
		    Capture_Command->Capture_433key_Judgetime=0;
		    real_tau0_ch7_width=0;
		    Flag.Need_433_data_compensate=0;
		}
		else
		{
		    Capture_Command->Capture_State=CAPTURE_HEAD_HIGH;
	            Capture_Command->Capture_Pulse_Wide_Sequence=0;
		}
	   }break;
	   */

	   case CAPTURE_ROLL_CODE://滚动码
	   {
		if(!_433_DATA)//本次读取的是高电平
		{
			if(g_tau0_ch7_width<INTERFERE_PLUSE_WIDTH)//认为是低电平受到干扰
			{
				Flag.Need_433_data_compensate=1;
				real_tau0_ch7_width+=g_tau0_ch7_width;
				if(Capture_Command->Capture_Pulse_Wide_Sequence>=2)
				{
					Capture_Command->Capture_Pulse_Wide_Sequence--;
				}
			}
			else
			{
				real_tau0_ch7_width=g_tau0_ch7_width;
				Flag.Need_433_data_compensate=0;
			}				
		}
		else//本次读取低电平
		{
			if(Flag.Need_433_data_compensate)
			{
				real_tau0_ch7_width+=g_tau0_ch7_width;
				if(Capture_Command->Capture_Pulse_Wide_Sequence>=2)
				{
					Capture_Command->Capture_Pulse_Wide_Sequence--;
				}
			}
			else
			{
				real_tau0_ch7_width=g_tau0_ch7_width;
			}
			Flag.Need_433_data_compensate=0;
		}	
	if(real_tau0_ch7_width>_800US_MAX_TIME)//判定出错
		{
	       Capture_Command->Capture_State=CAPTURE_HEAD_HIGH;
         Capture_Command->Capture_Pulse_Wide_Sequence=0;	
		}
		Capture_Command->Capture_Pulse_Wide_value[Capture_Command->Capture_Pulse_Wide_Sequence]=real_tau0_ch7_width;
		Capture_Command->Capture_Pulse_Wide_Sequence++;
		if(Capture_Command->Capture_Pulse_Wide_Sequence==ALL_CODE_LEVELS)//捕获129个电平时间,
		{
		     Capture_Command->Capture_State=CAPTURE_COMMANG_END;
             
		     Capture_Command->Capture_433key_Judgetime=0;
		}
	   }break;
	   case CAPTURE_COMMANG_END:
	   {
	   
	   }break;
	   default: break;
	 }
 

	}

uint8_t analysis_capture_command(CAPTURE_COMMAND *Capture_Command,uint8_t power_on_10stime_flag)//解析遥控器按键
{
  uint8_t length=0,i=0;
  uint8_t ret=FALSE;
  uint8_t _data[4];
  length = Capture_Command->Capture_Pulse_Wide_Sequence;  
  Capture_Command->Command_data=0;
  Capture_Command->Identify_data=0;
  Capture_Command->Secret_Key_data=0;
  for(i=2;i<length;i++)//解析收到的脉宽
  {
	if(i%2==0)
	{
	    if(i<=64)//识别码长度
	    {
	    	Capture_Command->Identify_data<<=1;
		if((_800US_MIN_TIME<=Capture_Command->Capture_Pulse_Wide_value[i])&&(_800US_MAX_TIME>=Capture_Command->Capture_Pulse_Wide_value[i]))
		{
		    Capture_Command->Identify_data|=1;    		
		}		
	    }
	    else
	    {
	    	Capture_Command->Secret_Key_data<<=1;
		if((_800US_MIN_TIME<=Capture_Command->Capture_Pulse_Wide_value[i])&&(_800US_MAX_TIME>=Capture_Command->Capture_Pulse_Wide_value[i]))
		{
		    Capture_Command->Secret_Key_data|=1;    		
		}			    
	    }
	}
  }
  Capture_Command->Command_data=Capture_Command->Identify_data&0XFF;
  Capture_Command->Identify_data&=0xffffff00;
  if(!Flag.Enter_Pcba_test_flag)//非自检
  {
	  if(power_on_10stime_flag&&Device_State_Data.Control_open==0)//上电10秒后
	  {	
		if(Capture_Command->Command_data==STUDY_CMD)
		{
			for(i=0;i<Flash_Data.Remote_Control_Num;i++)
			{
				if(Capture_Command->Identify_data==Flash_Data.Remote_Control_Id[i])
				{
					_data[0]=(uint8_t)(Capture_Command->Secret_Key_data>>24);
					_data[1]=(uint8_t)(Capture_Command->Secret_Key_data>>16);
					_data[2]=(uint8_t)(Capture_Command->Secret_Key_data>>8);
					_data[3]=(uint8_t)(Capture_Command->Secret_Key_data);		
					Keelop_KEY(0);//解密用的秘钥
			  		RF_DEC((uint8_t*)(&_data));//解密
					Capture_Command->Secret_Key_data=0;
					for(i=0;i<4;i++)
					{
						Capture_Command->Secret_Key_data<<=8;
						Capture_Command->Secret_Key_data+=_data[i];
					}
					Capture_Command->Capture_433key_Judgetime=0;
					Flag.Add_Match_Code_flag=0;
					ret=TRUE;
					break;
				}
			}
			//无此遥控器		
			if(i==Flash_Data.Remote_Control_Num)
			{
				 Flag.Add_Match_Code_flag=1;
				_data[0]=(uint8_t)(Capture_Command->Secret_Key_data>>24);
				_data[1]=(uint8_t)(Capture_Command->Secret_Key_data>>16);
				_data[2]=(uint8_t)(Capture_Command->Secret_Key_data>>8);
				_data[3]=(uint8_t)(Capture_Command->Secret_Key_data);
				Keelop_KEY(0);
				RF_DEC((uint8_t*)(&_data));
				Capture_Command->Secret_Key_data=0;
				for(i=0;i<4;i++)
				{
					Capture_Command->Secret_Key_data<<=8;
					Capture_Command->Secret_Key_data+=_data[i];
				}
				Capture_Command->Capture_433key_Judgetime=0;
				ret=TRUE;
			}
		}	
		else if(judge_remote_control(Capture_Command->Secret_Key_data))//遥控器码正确
		{			
			Capture_Command->Capture_433key_Judgetime=0;
			ret=TRUE;
		}
	  }
	  else
	  {
	  	if(Capture_Command->Command_data==STUDY_CMD)//上电 10S内添加遥控器
		{
			_data[0]=(uint8_t)(Capture_Command->Secret_Key_data>>24);
			_data[1]=(uint8_t)(Capture_Command->Secret_Key_data>>16);
	 		_data[2]=(uint8_t)(Capture_Command->Secret_Key_data>>8);
	  		_data[3]=(uint8_t)(Capture_Command->Secret_Key_data);
		  	Keelop_KEY(0);
		  	RF_DEC((uint8_t*)(&_data));//解密得到秘钥
			Capture_Command->Secret_Key_data=0;
			for(i=0;i<4;i++)
			{
				Capture_Command->Secret_Key_data<<=8;
				Capture_Command->Secret_Key_data+=_data[i];
			}
			Capture_Command->Capture_433key_Judgetime=0;
			ret=TRUE;	
		}
		else
		{
			if(judge_remote_control(Capture_Command->Secret_Key_data))//遥控器码正确
			{
				Capture_Command->Capture_433key_Judgetime=0;
				ret=TRUE;
			}
		}
	  }
  }
  else
  {
  	ret=TRUE;
  }
  return ret;
}

/*遥控器码响应*/
void capture_433key_judge_answer(button* handle,uint8_t power_on_10stime_flag)
{	
	static uint8_t cmd=0;	
	static uint16_t Capture_433KEY_LONG_TICKS;//对码长按时间
	if(Capture_Command.Capture_State==CAPTURE_COMMANG_END)//接收到一包遥控器数据
	{
		Capture_Command.Capture_State=CAPTURE_NO_DATA;				
		if(Capture_Command.Capture_433key_State==Capture_433KEY_NONE)
		{				
			if(analysis_capture_command((CAPTURE_COMMAND *)&Capture_Command,power_on_10stime_flag))//是否有正确遥控器包
			{			
				Capture_Command.Capture_433key_State=Capture_433KEY_PRESS;//收到按下按键发第一包数据			
				if(Capture_Command.Capture_433key_Cmd!=Capture_Command.Command_data)
				{
					Capture_Command.Capture_433key_Cmd=Capture_Command.Command_data;//赋值
					if(Capture_Command.Capture_433key_Cmd==SET_DOWN_LIMIT)
					{
						Capture_433KEY_LONG_TICKS=800;//设置限位命令长按5s
					}
					else if(Capture_Command.Capture_433key_Cmd==STUDY_CMD)
					{
						Capture_433KEY_LONG_TICKS=1350;//长按时间7s
					}
					else if(Capture_Command.Capture_433key_Cmd==MOTOR_DOWN_CMD)
					{
						Capture_433KEY_LONG_TICKS=1600;//长按时间10s
					}
					else
					{
						Capture_433KEY_LONG_TICKS=500;//长按时间3s
					}
				}			
			}	
			else
			{
				
			}
		}	
	}
	if(Capture_Command.Capture_433key_State==Capture_433KEY_PRESS)//遥控器按键按下
   	{
	    	if(Capture_Command.Capture_433key_Judgetime<Capture_433KEY_UP_JUDGE_TIME)
		{
			Capture_Command.Capture_433key_Judgetime++;
		}
		else//已经松手
		{
			Capture_Command.Capture_433key_Judgetime=0;//松手判定清0
			Capture_Command.Capture_433key_State=Capture_433KEY_UP;
		}
   	}
	if((handle->state) > 0) 
	{
		handle->ticks++;
	}
	if(Capture_Command.Capture_433key_State==Capture_433KEY_PRESS)//有遥控器按键按下
	{
		handle->button_level=1;
	}
	else if(Capture_Command.Capture_433key_State==Capture_433KEY_UP)//遥控器按键抬起
	{
		handle->button_level=0;	
	}
	switch(handle->state)
	{
		case 0:
		{
			if(handle->button_level==1)//有遥控器按键按下
			{
				handle->event = (uint8_t)PRESS_DOWN;
				handle->ticks  = 0;
				handle->repeat = 1;
				handle->state  = 1; 
			}
			else
			{
				handle->event = (uint8_t)NONE_PRESS;
			}
		}break;
		case 1:
		{
			if(handle->button_level==0)//遥控器按键抬起
		        { 
			   	Capture_Command.Capture_433key_State=Capture_433KEY_NONE;	
				handle->event = (uint8_t)PRESS_UP;		   
		                handle->ticks = 0;
		                handle->state = 2;                                                                                                                                                                                          		
		        } 
		        else if(handle->ticks > Capture_433KEY_LONG_TICKS)//按键一直按下一定时长
		        {
		        	handle->event = (uint8_t)LONG_RRESS_START;
		        	handle->state = 5; //长按状态
		        }
		}break;
		case 2:
		{
		        if(handle->button_level==1)//连续有按键按下
		        {             
			    handle->event = (uint8_t)PRESS_DOWN;
		            handle->ticks = 0;
		            handle->state = 3;
		        } 
		        else //遥控器按键抬起
			{	
				Capture_Command.Capture_433key_State=Capture_433KEY_NONE;
			}
		
		}break;
		case 3:
		{
		        if(handle->button_level==0) //有按键抬起
		        { 
				Capture_Command.Capture_433key_State=Capture_433KEY_NONE;
				handle->event = (uint8_t)PRESS_UP;					           	
				handle->state = 0;
		        }
			else if(handle->ticks > Capture_433KEY_LONG_TICKS)//按键一直按下一定时长
		        {
		            handle->event = (uint8_t)LONG_RRESS_START;
		            handle->state = 5; //长按状态
		        }
		}break;
       		case 5:
		{
		        if(handle->button_level==1)//按键长按保持
		        {
		            handle->event = (uint8_t)LONG_PRESS_HOLD;
		        } 
		        else 
		        {	
			    Capture_Command.Capture_433key_State=Capture_433KEY_NONE;
		            handle->event = (uint8_t)LONG_PRESS_UP;        
		            handle->state = 0; 
		        }
		}break;	
		default:break;
	}	
	cmd=handle->event;//得到按键状态
	handle->event=NONE_PRESS;
	if(cmd!=NONE_PRESS)//遥控器按键抬起
	{
		if(cmd==PRESS_DOWN)//遥控器按键按下
		{
			if(Capture_Command.Capture_433key_Cmd!=STUDY_CMD)
			{
				Study_Cmd_Repeat=0;
			}
			if(Capture_Command.Capture_433key_Cmd!=MOTOR_STOP_CMD)
			{
				STOP_CMD_Repeat=0;
			}
			switch(Capture_Command.Capture_433key_Cmd)
			{
				case MOTOR_UP_CMD:
				{
					Cmd_Motor_Up();
					Flag.No_judge_electronic_limit=0;
				}break;  
				case MOTOR_DOWN_CMD:
				{
				       /*if((Err.err_bit.Encounter_Obstacle)&&(Device_State_Data.Motor_Position<10))
				       {
				       		Flag.Motor_force_down=1;
						Device_State_Data.Motor_State_Byte=MOTOR_DOWN_STATE;
						SetupBzhx(MOTOR_DOWN_BEEP,Flash_Data.Beep_open);
						Flag.Motor_run_now=0;
						Time.Motor_Force_Down_time=0;
						Time.Motor_Stop_Encounter_Obstacle_time=0;//不做遇阻判断，判断时间清0
						current_state=5;  
				       }
				       else
					{
						Cmd_Motor_Down();
					}*/
					Cmd_Motor_Down();
					Flag.No_judge_electronic_limit=0;
				}break;
				case MOTOR_STOP_CMD:
				{	

					Cmd_Motor_Stop();
					Flag.No_judge_electronic_limit=0;
					if(power_on_10stime_flag)
					{
						if(Err.err_bit.Encounter_Obstacle)
						{
							STOP_CMD_Repeat++;
							if(STOP_CMD_Repeat<=2)
							{
								SetupBzhx(SHORT_BEEP,Flash_Data.Beep_open);
							}
							else if(STOP_CMD_Repeat==3)//清码
							{
								SetupBzhx(LONG_BEEP,1);
								match_data_clear();
								STOP_CMD_Repeat=0;
							}
							Time.Del_Match_Code_time=0;
						}
						else
						{
							STOP_CMD_Repeat=0;
						}
					}
				}break;
				case LIGHT_CMD:
				{	
					Cmd_Light();
				}break; 
				case STUDY_CMD:
				{	
					
					if(Device_State_Data.Control_open)
					{
							SetupBzhx(CODE_MATCH_BEEP, 1);
							match_data_del();
							break ;  
					}
					if(power_on_10stime_flag)//上电10s后
					{	
						if(Flag.Add_Match_Code_flag)
						{
							if(Err.err_bit.Encounter_Obstacle)
							{
								SetupBzhx(CODE_MATCH_BEEP,1);//添码
								match_data_del();
							}
						}
						else
						{
							SetupBzhx(SHORT_BEEP,Flash_Data.Beep_open);	
						}
						
					}
					else//执行对码
					{
						SetupBzhx(CODE_MATCH_BEEP,1);
						match_data_del();
					}			
			
				}break;
				case OVEREIGHT_CURRENT_CMD://超重电流校正
				{
					if((Device_State_Data.Motor_State_Byte==MOTOR_SUSPEND_STATE)
					||(Device_State_Data.Motor_State_Byte==MOTOR_DOWN_STOP_STATE))//当前是暂停状态
					{
						Flag.Overweight_current_correct_start=1;
						Time.Overweight_Current_time=0;
					//	Overweight_correct.Sample_num=0;
					//	Overweight_correct.Sample_sum=0;
						SetupBzhx(MOTOR_UP_BEEP,Flash_Data.Beep_open);
						Device_State_Data.Motor_State_Byte=MOTOR_UP_STATE;
						Flag.Motor_run_now=0;	
					}
				}break;
				default:break;
			}
		}
		else if(cmd==LONG_RRESS_START)//遥控器长按
		{
			switch(Capture_Command.Capture_433key_Cmd)
		  	{	
			      case STUDY_CMD:
			      {
				      if((power_on_10stime_flag)&&(!Flag.Add_Match_Code_flag))//上电10s后
				      {
					Study_Cmd_Repeat=0;
				      	SetupBzhx(WIFI_RESET_BEEP,1);
					ModuleResetReq();
					Led.Light_Breathing_time=0;
					if(brightness_temp>50)
					{
						Led.Light_Led_state=2;//呼吸灯灭
					}
					else
					{
						Led.Light_Led_state=1;//呼吸灯亮
					}
					Light_colour_temperature_gear=540;//重置后色温为最冷
				      }
			      }break;
			      case MOTOR_DOWN_CMD:
			      {
			      	       if((Err.err_bit.Encounter_Obstacle)&&((Device_State_Data.Motor_Position<30)||(Device_State_Data.Motor_Position==100)))
				       {
				       		Flag.Motor_force_down=1;
						Device_State_Data.Motor_State_Byte=MOTOR_DOWN_STATE;
						SetupBzhx(MOTOR_DOWN_BEEP,Flash_Data.Beep_open);
						Flag.Motor_run_now=0;
						Time.Motor_Force_Down_time=0;
						Time.Motor_Stop_Encounter_Obstacle_time=0;//不做遇阻判断，判断时间清0
						current_state=5;  		
				       }   
			      }break;
			      default:break;
			}
		}
		cmd=NONE_PRESS;
	}
}