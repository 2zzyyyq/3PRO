#include "ALL.h" 

uint8_t ADC_sample_num, min_serial_num, max_serial_num;
uint8_t ADC_sample_num_B, min_serial_num_B, max_serial_num_B;
uint8_t ADC_sample_num_yz, min_serial_num_yz, max_serial_num_yz;
uint8_t ADC_sample_num_yz_B, min_serial_num_yz_B, max_serial_num_yz_B;
uint16_t ADC_sample_max, ADC_sample_min, ADC_sample_max_yz, ADC_sample_min_yz;
uint16_t ADC_sample_max_B, ADC_sample_min_B, ADC_sample_max_yz_B, ADC_sample_min_yz_B;
uint16_t ADC_read_buf[AD_BUFF_DIP], ADC_read_buf_B[AD_BUFF_DIP];
uint16_t  value,temp;
int16_t  offset;
float over_weight_current;
float addvalue_A;
uint16_t motor_current_to_advalue(float  circle);

void Ovreweight_current_correct(void)//超重按整机校正，两杆均匀分布挂重，各挂20kg
{
	temp=OVER_WIGHT_CURRENT*100;//每根晾杆上按承受一半重量的电流
	if(Flag.Overweight_current_correct_start)
	{
		if(Time.Overweight_Current_time>60)
		{
			Time.Overweight_Current_time=0;
			Flag.Overweight_current_correct_start=0;
			Overweight_Correct.Sample_ave=Overweight_Correct.Sample_sum/Overweight_Correct.Sample_num;
			Overweight_Correct.sample_current=Overweight_Correct.Sample_ave*1500/4096;//放大100倍		
			offset=Overweight_Correct.sample_current-temp;
			if((offset>-50)&&(offset<50))//校正完成
			{
				Flash_Data.Overweight_Correct=125+offset;
				Ell_Data.Motor_Run_Circle=0;
		    save_data();
				Device_State_Data.Motor_State_Byte=MOTOR_SUSPEND_STATE;
				Flag.Motor_run_now=0;
				SetupBzhx(MOTOR_STOP_BEEP,Flash_Data.Beep_open);
			}
			else//待重新校正
			{
				Device_State_Data.Motor_State_Byte=MOTOR_DOWN_STATE;
		
				Flag.Motor_up_to_down=1;
				Flag.Motor_run_now=0;
			}	
		}
	}
//	if(Flag.B_Overweight_current_correct_start)
//	{
//		if(Time.B_Overweight_Current_time>60)
//		{
//			Time.B_Overweight_Current_time=0;
//			Flag.B_Overweight_current_correct_start=0;
//			B_Overweight_Correct.Sample_ave=B_Overweight_Correct.Sample_sum/B_Overweight_Correct.Sample_num;
//			B_Overweight_Correct.sample_current=B_Overweight_Correct.Sample_ave*1500/4096;//放大100倍		  1.5为参考电压
//			offset=B_Overweight_Correct.sample_current-temp;
//			if((offset>-50)&&(offset<50))//校正完成
//			{
//				Flash_Data.B_Overweight_Correct=125+offset;
//				Ell_Data.B_Motor_Run_Circle=0;	
//      	save_data();
//				Device_State_Data.B_Motor_State_Byte=MOTOR_SUSPEND_STATE;
//				Flag.B_Motor_run_now=0;
//				SetupBzhx(MOTOR_STOP_BEEP,Flash_Data.Beep_open);
//			}
//			else//待重新校正
//			{
//				Device_State_Data.B_Motor_State_Byte=MOTOR_DOWN_STATE;
//		
//				Flag.B_Motor_up_to_down=1;
//				Flag.B_Motor_run_now=0;
//			}	
//		}
//	}
}
/*****************************************************************************
转换对应超重电流AD值
Function name:motor_current_to_advalue
Input parameters: circle-->motor run  circle 电机运行周期：1次对应10个循环
*****************************************************************************/
uint16_t motor_current_to_advalue(float  circle)
{
	over_weight_current=OVER_WIGHT_CURRENT+((float)Flash_Data.Overweight_Correct-125)/100;
	circle=circle*1.0/100;//以1000次为单位
	//return ((over_weight_current+0.2*circle)*0.1/1.45)*1024;//超重电流对应AD值,0.2为补偿电流
	return ((over_weight_current+0.2*circle)*0.1/3.6)*4096;//超重电流对应AD    --497值,  0.2为补偿电流   1.5为参考电压
}






























































































































