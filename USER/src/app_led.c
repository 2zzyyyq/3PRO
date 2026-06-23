#include "ALL.h" 
uint16_t  Light_colour_temperature_gear=0;
__IO uint16_t   colour_temperature_temp=0;
uint16_t  brightness_target,TDR07_target_value,TDR05_target_value,brightness_temp;
uint8_t   step;	
uint8_t   step_value;
uint8_t key;
LED Led ;
#define NO_KEY 0X01
const uint16_t  LIGHE_COLOR[3]={5700,4000,3000};
//PA08		MOR2_H2	电机2霍尔信号
//PA09		MOR2_H1
//PA10		MOR1_H2	电机1霍尔信号
//PA11		MOR1_H1
uint8_t light_key_time;//照明键连续按下次数

void Cmd_Light(void)
{
   // Led.Light_Led_state=0;//有操作照明键，退出led指示状态
 if(Device_State_Data.Light_State_Byte==LIGHT_OFF)//当前关
	{
		Device_State_Data.Light_State_Byte=LIGHT_ON;
		SetupBzhx(LIGHT_ON_BEEP,Flash_Data.Beep_open);
		if(Time.Light_key_space_time<Light_KEY_SPACE_TIME)
		{
			if(Device_State_Data.Light_colour_temperature==0)
			{
				light_key_time=0;	
			}
			Device_State_Data.Light_colour_temperature=LIGHE_COLOR[light_key_time];
			Flash_Data.Light_CT_value=Device_State_Data.Light_colour_temperature;
		 	light_key_time++;
			if(light_key_time>=3)
			{
				light_key_time=0;
			}
		}
		colour_temperature_temp=(Flash_Data.Light_CT_value-3000)/5;//开灯时色温不渐变，直接是最终状态
		if(Flag.Brightness_time_flag)
		{
			Device_State_Data.Light_Bright=Flash_Data.Light_Bright_Value;
		}
		else
		{
			Device_State_Data.Light_Bright=Flash_Data.Max_Light_Value;
		}
	}
	else if(Device_State_Data.Light_State_Byte==LIGHT_ON)//当前开
	{
		Device_State_Data.Light_State_Byte=LIGHT_OFF;
		Flag.Brightness_change=0;
		Flag.Auto_light=0;
		SetupBzhx(LIGHT_OFF_BEEP,Flash_Data.Beep_open);
		Time.Light_key_space_time=0;
	}	
}
/*
 * light_work() — LED dual-color PWM control (ISR context, called every 10ms)
 *
 * Integer-only arithmetic replaces all float ops to avoid software FPU library
 * stack pollution on Cortex-M0+ (no hardware FPU).  Float emulation in ISR
 * context was corrupting the uart1_tx_buf region via stack overflow into .bss.
 *
 * Formulas (integer equivalents):
 *   White PWM = brightness_factor * CT_gear * 3 / 100
 *   Yellow PWM = brightness_factor * (540 - CT_gear) * 3 / 100
 *   where brightness_factor = brightness * 0.95 + 5  →  (brightness * 95 + 500) / 100
 *   (scaled: multiply all by 100, then divide by 10000)
 *
 * Final simplified form:
 *   White  = (brightness * 95 + 500) * CT_gear * 3 / 10000
 *   Yellow = (brightness * 95 + 500) * (540 - CT_gear) * 3 / 10000
 */
void light_work(uint8_t light_state)
{
	uint16_t bt;
	uint32_t factor;

	brightness_target = Device_State_Data.Light_Bright;
	Light_colour_temperature_gear = (Device_State_Data.Light_colour_temperature - 3000) / 5;

	switch (light_state)
	{
	case 0:  /* fade out */
		if (brightness_temp > 0)
		{
			brightness_temp--;
			LIGHT_WHITE_VALUE  = (uint16_t)((uint32_t)brightness_temp * Light_colour_temperature_gear * 3 / 100);
			LIGHT_YELLOW_VALUE = (uint16_t)((uint32_t)brightness_temp * (540 - Light_colour_temperature_gear) * 3 / 100);
		}
		else
		{
			LED_LIGHT_OFF;
		}
		break;

	case 1:  /* fade in + color temperature ramp */
		LED_LIGHT_ON;

		if (brightness_temp != brightness_target)
		{
			if (brightness_temp < brightness_target)
				brightness_temp++;
			else
				brightness_temp--;

			bt     = brightness_temp;
			factor = (uint32_t)bt * 95 + 500;
			LIGHT_WHITE_VALUE  = (uint16_t)(factor * Light_colour_temperature_gear * 3 / 10000);
			LIGHT_YELLOW_VALUE = (uint16_t)(factor * (540 - Light_colour_temperature_gear) * 3 / 10000);
		}
		else if (colour_temperature_temp != Light_colour_temperature_gear)
		{
			bt     = Device_State_Data.Light_Bright;
			factor = (uint32_t)bt * 95 + 500;

			if (colour_temperature_temp < Light_colour_temperature_gear)
			{
				if      (Light_colour_temperature_gear < 25)  step_value = 25;
				else if (Light_colour_temperature_gear < 100) step_value = 20;
				else if (Light_colour_temperature_gear < 200) step_value = 15;
				else if (Light_colour_temperature_gear < 400) step_value = 10;
				else                                          step_value = 5;

				if (colour_temperature_temp > 540)
				{
					uint16_t tgt_white  = (uint16_t)(factor * Light_colour_temperature_gear * 3 / 10000);
					uint16_t tgt_yellow = (uint16_t)(factor * (540 - Light_colour_temperature_gear) * 3 / 10000);
					if      (LIGHT_WHITE_VALUE  < tgt_white)  LIGHT_WHITE_VALUE++;
					else if (LIGHT_WHITE_VALUE  > tgt_white)  LIGHT_WHITE_VALUE--;
					if      (LIGHT_YELLOW_VALUE < tgt_yellow) LIGHT_YELLOW_VALUE++;
					else if (LIGHT_YELLOW_VALUE > tgt_yellow) LIGHT_YELLOW_VALUE--;
					if (LIGHT_WHITE_VALUE == tgt_white || LIGHT_YELLOW_VALUE == tgt_yellow)
						colour_temperature_temp = Light_colour_temperature_gear;
				}
				else
				{
					colour_temperature_temp += step_value;
					if (colour_temperature_temp > Light_colour_temperature_gear)
						colour_temperature_temp = Light_colour_temperature_gear;
					LIGHT_WHITE_VALUE  = (uint16_t)(factor * colour_temperature_temp * 3 / 10000);
					LIGHT_YELLOW_VALUE = (uint16_t)(factor * (540 - colour_temperature_temp) * 3 / 10000);
				}
			}
			else
			{
				if      (Light_colour_temperature_gear < 25)  step_value = 5;
				else if (Light_colour_temperature_gear < 100) step_value = 10;
				else if (Light_colour_temperature_gear < 200) step_value = 15;
				else if (Light_colour_temperature_gear < 400) step_value = 20;
				else                                          step_value = 25;

				if (colour_temperature_temp < 25)
				{
					uint16_t tgt_white  = (uint16_t)(factor * Light_colour_temperature_gear * 3 / 10000);
					uint16_t tgt_yellow = (uint16_t)(factor * (540 - Light_colour_temperature_gear) * 3 / 10000);
					if      (LIGHT_WHITE_VALUE  < tgt_white)  LIGHT_WHITE_VALUE++;
					else if (LIGHT_WHITE_VALUE  > tgt_white)  LIGHT_WHITE_VALUE--;
					if      (LIGHT_YELLOW_VALUE < tgt_yellow) LIGHT_YELLOW_VALUE++;
					else if (LIGHT_YELLOW_VALUE > tgt_yellow) LIGHT_YELLOW_VALUE--;
					if (LIGHT_WHITE_VALUE == tgt_white || LIGHT_YELLOW_VALUE == tgt_yellow)
						colour_temperature_temp = Light_colour_temperature_gear;
				}
				else
				{
					if (colour_temperature_temp > Light_colour_temperature_gear + step_value)
						colour_temperature_temp -= step_value;
					else
						colour_temperature_temp = Light_colour_temperature_gear;
					LIGHT_WHITE_VALUE  = (uint16_t)(factor * colour_temperature_temp * 3 / 10000);
					LIGHT_YELLOW_VALUE = (uint16_t)(factor * (540 - Light_colour_temperature_gear) * 3 / 10000);
				}
			}
		}
		break;

	default:
		break;
	}
}


void led_work(void)
{
	if(Led.Led_Orange_state==LED_ORG_STATE_TWINK)//橙灯闪烁状态
	{
		Led.Led_Orange_On_time++;	
	}
	
	if(Led.Led_Blue_state==LED_BLUE_STATE_ON)//蓝灯常亮状态
	{
		Led.Led_Blue_On_time++;
	}
	
	if(Led.Led_Orange_On_time>=LED_ORG_TWINKLE_TIME)//3min钟到
	{
		Led.Led_Orange_state=LED_ORG_STATE_OFF;
		Led.Led_Orange_On_time=0;
	}
	if(Led.Led_Blue_On_time>=LED_BLUE_ON_TIME)//3min钟到
	{
		Led.Led_Blue_state=LED_BLUE_STATE_OFF;
		Led.Led_Blue_On_time=0;
	}
	if(Led.Led_Twinkle_time==0)
	{
		if(Led.Led_Orange_state==LED_ORG_STATE_OFF)//橙灯熄灭状态
		{
 	   LED_ORANGE_OFF;			
		}
		else
		{
		 LED_ORANGE_ON;	
		}		
		if(Led.Led_Blue_state==LED_BLUE_STATE_OFF)//蓝灯熄灭状态
		{
		 LED_BLUE_OFF;
		}
		else
		{
		 LED_BLUE_ON;
		}
	}
	else if (Led.Led_Twinkle_time==LED_TWINKLE_ON_TIME)
	{
	   LED_ORANGE_OFF;	
		if(Led.Led_Blue_state==LED_BLUE_STATE_ON)//蓝灯常亮状态
		{
         	LED_BLUE_ON;
		}
		else
		{
	       LED_BLUE_OFF;
		}
	}
	Led.Led_Twinkle_time++;
	if(Led.Led_Twinkle_time==LED_TWINKLE_TIME)
	{
		Led.Led_Twinkle_time=0;
	}
}













































