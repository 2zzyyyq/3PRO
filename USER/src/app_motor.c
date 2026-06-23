#include "ALL.h"
#include <math.h>  // kept for compatibility; sinf() replaced by lookup table in Motor_speed()
volatile ELL_DATA  Ell_Data;
volatile DEVICE_STATE_DATA   Device_State_Data;
volatile DEVICE_STATE_DATA   Up_State_Data;
ERR Err,Err_B;
uint8_t   Soft_start_moveA;
uint8_t   current_state;
uint16_t  set_up_limit_value;
uint16_t  set_down_limit_value;
volatile uint8_t Holl_Circle_num, Motor_Circle_num;
uint16_t Holl_pulse_temp;
volatile int Motor_Current_Position_pulse, Holl_test_speed_temp;
uint16_t set_position_pulse;
uint8_t Net_state;
uint8_t Flag_TOP_ods;
uint16_t target_position;
uint8_t Self_Inspection_step;
static uint8_t last_motor_state=0;
void Cmd_Motor_Up(void);
void Cmd_Motor_Down(void);
void Cmd_Motor_Stop(void);
void Cmd_Motor_Up_Longpress(void);
void Delay1ms(uint32_t nTime);

void Cmd_Motor_Down(void)
{
	Flag.Run_to_set_position_flag=0;
	Err.err_bit.Motor_Obs=0;
	if((Flag.Motor_down_to_up==0)&&(Flag.Motor_up_to_down==0))
	{
		if(Err.err_bit.Encounter_Obstacle==1)
		{
			Err.err_bit.Encounter_Obstacle=0;
			Time.Encounter_Obstacle_beepnum=0;
			Up_State_Data.Err_Byte&=0xfe;
		}
		else
		{
			if((!Flag.Down_limit)&&((!(Ell_Data.Motor_Current_Position>=set_down_limit_value))||Flag.No_judge_electronic_limit))
			{
				if((Device_State_Data.Motor_State_Byte==MOTOR_UP_STOP_STATE)
			      	||(Device_State_Data.Motor_State_Byte==MOTOR_SUSPEND_STATE)
				||(Device_State_Data.Motor_State_Byte==MOTOR_DOWN_STOP_STATE))
				{
					if((Device_State_Data.Motor_Position>=Device_State_Data.electronic_down_limit)&&(!Flag.No_judge_electronic_limit))
					{

					}
					else
					{
						Device_State_Data.Motor_State_Byte=MOTOR_DOWN_STATE;
						SetupBzhx(MOTOR_DOWN_BEEP,Flash_Data.Beep_open);
						Flag.Motor_run_now=0;
					}
				}
				else if(Device_State_Data.Motor_State_Byte==MOTOR_UP_STATE)
				{
					Device_State_Data.Motor_State_Byte=MOTOR_DOWN_STATE;
					Flag.Motor_up_to_down=1;
					Flag.Motor_run_now=0;
				}
				else if (Device_State_Data.Motor_State_Byte==MOTOR_DOWN_STATE)
				{
					Device_State_Data.Motor_State_Byte=MOTOR_SUSPEND_STATE;
					SetupBzhx(MOTOR_STOP_BEEP,Flash_Data.Beep_open);
					Flag.Motor_run_now=0;
				}
			}
		}
	}
}

void Cmd_Motor_Stop(void)
{
	Flag.Run_to_set_position_flag=0;
	Err.err_bit.Motor_Obs=0;
	if((Flag.Motor_down_to_up==0)&&(Flag.Motor_up_to_down==0))
	{
		if((Device_State_Data.Motor_State_Byte==MOTOR_UP_STOP_STATE)
		||(Device_State_Data.Motor_State_Byte==MOTOR_DOWN_STOP_STATE))
		{

		}
		else if(Device_State_Data.Motor_State_Byte==MOTOR_UP_STATE)
		{
			if(Flag.Motor_down_to_up!=1)
			{
				Device_State_Data.Motor_State_Byte=MOTOR_SUSPEND_STATE;
				Flag.Motor_run_now=0;
				SetupBzhx(MOTOR_STOP_BEEP,Flash_Data.Beep_open);
			}
		}
		else if(Device_State_Data.Motor_State_Byte==MOTOR_DOWN_STATE)
		{
			if(Flag.Motor_up_to_down!=1)
			{
				Device_State_Data.Motor_State_Byte=MOTOR_SUSPEND_STATE;
				Flag.Motor_run_now=0;
				SetupBzhx(MOTOR_STOP_BEEP,Flash_Data.Beep_open);
			}
		}
	}
}


void motor_stop(void)
{
	#ifdef NO_PWM_MOTOR
	  	MOTOR_DN_IO(1);

		  MOTOR_UP_IO(1);
	#else

		 Soft_start_moveA=PWM_STOP;
		#endif
}
void motor_up(void)
{
	#ifdef NO_PWM_MOTOR
    MOTOR_DN_IO(1);
	 	MOTOR_UP_IO(0);
	#else
	Soft_start_moveA=PWM_UP;

	#endif

}

void motor_down(void)
{
	#ifdef NO_PWM_MOTOR
 	    MOTOR_DN_IO(0);
	   	MOTOR_UP_IO(1);
	#else

		Soft_start_moveA=PWM_DOWN;
	#endif

}

void Delay1ms(uint32_t nTime)
{
	uint32_t i , j;
	for (j = nTime; j > 0; j--)
	{
		for (i = 0; i < 4800; i++);
	}
}


void motor_work(uint8_t motor_state)
{
	switch(motor_state)
	{
		case MOTOR_UP_STOP_STATE:
		case MOTOR_DOWN_STOP_STATE:
		case MOTOR_SUSPEND_STATE:
		{
			Flag.Enable_Test_Over_Wight_flag=0;
			Flag.Enable_Test_Motor_Obs_flag=0;
			motor_stop();
			Flag.No_judge_electronic_limit=0;
			Flag.Motor_run_now=0;
			Time.Motor_Run_time=0;
			Flag.Motor_Start_up=0;
			Time.Motor_Start_Up_time=0;
			Holl_pulse_temp=Ell_Data.Motor_Current_Position;
			Flag.Overweight_current_correct_start=0;
		}break;
		case MOTOR_UP_STATE:
		{
			if((!Flag.Motor_down_to_up)&&(!Flag.Motor_raise_up))
			{
				if(!Flag.Enable_Test_Over_Wight_flag)
				{
					Flag.Motor_Start_up=1;
				}
				motor_up();
				Time.Over_Wight_beepnum=ALARM_OVER_WEIGHT_BEEP_NUM;
				Time.Motor_Err_beepnum=ALARM_MOTOR_BEEP_NUM;
				if(!Flag.Motor_run_now)
				{
					Time.Motor_Run_time=0;
					Holl_pulse_temp=Ell_Data.Motor_Current_Position;
				}
				Flag.Motor_run_now=1;
			}
			else
			{
				motor_stop();
				Flag.Motor_run_now=0;
				Time.Motor_Run_time=0;
				Flag.Motor_Start_up=0;
				Time.Motor_Start_Up_time=0;
				Holl_pulse_temp=Ell_Data.Motor_Current_Position;
			}
		}break;
		case MOTOR_DOWN_STATE:
		{
			Flag.Enable_Test_Motor_Obs_flag=0;
			Flag.Enable_Test_Over_Wight_flag=0;
			if(!Flag.Motor_up_to_down)
			{
				motor_down();
				Time.Motor_Err_beepnum=ALARM_MOTOR_BEEP_NUM;
				if(!Flag.Motor_run_now)
				{
					Time.Motor_Run_time=0;
					Holl_pulse_temp=Ell_Data.Motor_Current_Position;
				}
				Flag.Motor_run_now=1;
			}
			else
			{
				motor_stop();
				Time.Motor_Run_time=0;
				Flag.Motor_run_now=0;
				Flag.Motor_Start_up=0;
				Time.Motor_Start_Up_time=0;
				Holl_pulse_temp=Ell_Data.Motor_Current_Position;
			}
		}break;
	}
}

void Cmd_Motor_Up(void)
{
	Flag.Run_to_set_position_flag=0;
	Err.err_bit.Motor_Obs=0;
	if((Flag.Motor_down_to_up==0)&&(Flag.Motor_up_to_down==0))
	{
		if(Err.err_bit.Encounter_Obstacle==1)
		{
			Err.err_bit.Encounter_Obstacle=0;
			Time.Encounter_Obstacle_beepnum=0;
			Up_State_Data.Err_Byte&=0xfe;
		}
		else
		{
			if((!Flag.Up_limit)&&((!(Ell_Data.Motor_Current_Position<=set_up_limit_value)||Flag.No_judge_electronic_limit))&&(current_state==0))
			{
				if((Device_State_Data.Motor_State_Byte==MOTOR_UP_STOP_STATE)
			      	||(Device_State_Data.Motor_State_Byte==MOTOR_SUSPEND_STATE)
				||(Device_State_Data.Motor_State_Byte==MOTOR_DOWN_STOP_STATE))
				{
					SetupBzhx(MOTOR_UP_BEEP,Flash_Data.Beep_open);
					Device_State_Data.Motor_State_Byte=MOTOR_UP_STATE;
					Flag.Motor_run_now=0;
				}
				else if(Device_State_Data.Motor_State_Byte==MOTOR_UP_STATE)
				{
					SetupBzhx(MOTOR_STOP_BEEP,Flash_Data.Beep_open);
					Device_State_Data.Motor_State_Byte=MOTOR_SUSPEND_STATE;
					Flag.Motor_run_now=0;
				}
				else if(Device_State_Data.Motor_State_Byte==MOTOR_DOWN_STATE)
				{
					Device_State_Data.Motor_State_Byte=MOTOR_UP_STATE;
					Flag.Motor_run_now=0;
					Flag.Motor_down_to_up=1;
				}
			}
		}
	}
}

void Cmd_Motor_Up_Longpress(void)
{

	Flag.Run_to_set_position_flag=0;
	Err.err_bit.Motor_Obs=0;
	if((Flag.Motor_down_to_up==0)&&(Flag.Motor_up_to_down==0))
	{
		if(Err.err_bit.Encounter_Obstacle==1)
		{
			Err.err_bit.Encounter_Obstacle=0;
			Time.Encounter_Obstacle_beepnum=0;
			Up_State_Data.Err_Byte&=0xfe;
		}
		else
		{
			{
				if((Device_State_Data.Motor_State_Byte==MOTOR_UP_STOP_STATE)
			      	||(Device_State_Data.Motor_State_Byte==MOTOR_SUSPEND_STATE)
				||(Device_State_Data.Motor_State_Byte==MOTOR_DOWN_STOP_STATE))
				{
					SetupBzhx(MOTOR_UP_BEEP,Flash_Data.Beep_open);
					Device_State_Data.Motor_State_Byte=MOTOR_UP_STATE;
					Flag.Motor_run_now=0;
					Flag.Run_to_set_position_flag=1;
					set_position_pulse=0;
					Device_State_Data.convergent=CONVERGENT_ON;


				}
				else if(Device_State_Data.Motor_State_Byte==MOTOR_DOWN_STATE)
				{
					Device_State_Data.Motor_State_Byte=MOTOR_UP_STATE;
					Flag.Motor_run_now=0;
					Flag.Motor_down_to_up=1;
					Flag.Run_to_set_position_flag=1;
					set_position_pulse=0;
					Device_State_Data.convergent=CONVERGENT_ON;

				}
			}
		}
	}
}





/* ==================================================================
 * Motor_speed()  —  H-bridge soft-start / soft-stop (sine curve)
 *
 * Called every 5ms from main loop (Flag.time5ms_flag).
 *
 * OPTIMIZED version:
 *   - Pre-computed integer sin lookup table replaces float sinf()
 *     (Cortex-M0+ has no hardware FPU; sinf() costs ~200 cycles/call)
 *   - pwmModeActive flag avoids redundant TIM_OC mode register writes
 *   - pwmDuty calculated only when stepCounter changes
 *   - Obstacle-triggered stop handled with early return
 *
 * Start: pwmDuty ramps 1600→0 following 1600*(1-sin(step/100 * pi/2))
 * Stop:  pwmDuty ramps 0→1600 following 1600*(1-sin(step/100 * pi/2))
 * ================================================================== */

/* sinPwmTable[i] = round(1600 * sin(i/100 * pi/2)), i = 0..100 */
static const uint16_t sinPwmTable[101] = {
       0,  25,  50,  75, 100, 126, 151, 176, 201, 225,
     250, 275, 300, 324, 349, 374, 398, 422, 446, 470,
     494, 518, 542, 566, 589, 612, 635, 658, 681, 704,
     726, 749, 771, 793, 814, 836, 857, 878, 899, 920,
     940, 961, 981,1000,1020,1039,1058,1077,1095,1113,
    1131,1149,1166,1183,1200,1217,1233,1249,1264,1280,
    1294,1309,1323,1337,1351,1364,1377,1390,1402,1414,
    1426,1437,1448,1458,1468,1478,1488,1497,1505,1514,
    1522,1529,1536,1543,1550,1556,1561,1567,1572,1576,
    1580,1584,1587,1590,1593,1595,1597,1598,1599,1600,
    1600
};

void Motor_speed(void)
{
    static uint16_t stepCounter = 0;
    static const uint16_t maxSteps = PWM_MAX_COMPARE;   /* = 100 */
    static uint8_t  pwmModeActive = 0;                   /* cache: PWM_DOWN/PWM_UP/0 */
    uint16_t pwmDuty;

    /* ====== START phase: ramp up following sin(0 -> pi/2) ====== */
    if ((Soft_start_moveA == PWM_DOWN || Soft_start_moveA == PWM_UP)
        && stepCounter <= maxSteps)
    {
        /* advance step (duty is calculated AFTER increment so first output uses step=2) */
        if (stepCounter < maxSteps)
        {
            stepCounter += PWM_START_SPEED;
            if (stepCounter > maxSteps)
            {
                stepCounter = maxSteps;
            }
        }

        if (stepCounter >= maxSteps)
        {
            /* ramp complete — bypass PWM, force 100% duty */
            if (Soft_start_moveA == PWM_DOWN)
            {
                PWM_FORCE_CH2_HIGH_CH3_LOW();
            }
            else
            {
                PWM_FORCE_CH3_HIGH_CH2_LOW();
            }
            pwmModeActive = 0;
        }
        else
        {
            /* pwmDuty = 1600 - sinTable[step]  →  ramps 1600 down to 0 */
            pwmDuty = PWM_MAX_VALUE - sinPwmTable[stepCounter];

            if (Soft_start_moveA == PWM_DOWN)
            {
                if (pwmModeActive != PWM_DOWN)
                {
                    TIM_SET_OC3_PWM(TIM_OUTPWM);
                    TIM_SET_OC4_HIGH(TIM_OUTPWM);
                    pwmModeActive = PWM_DOWN;
                }
                TIM_SetCmp3(TIM_OUTPWM, pwmDuty);
                last_motor_state = PWM_DOWN;
            }
            else  /* PWM_UP */
            {
                if (pwmModeActive != PWM_UP)
                {
                    TIM_SET_OC4_PWM(TIM_OUTPWM);
                    TIM_SET_OC3_HIGH(TIM_OUTPWM);
                    pwmModeActive = PWM_UP;
                }
                TIM_SetCmp4(TIM_OUTPWM, pwmDuty);
                last_motor_state = PWM_UP;
            }
        }
    }

    /* ====== STOP phase: ramp down following sin(pi/2 -> 0) ====== */
    if (Soft_start_moveA == PWM_STOP)
    {
        /* obstacle-triggered immediate stop */
        if (Err.err_bit.Motor_Obs == 1)
        {
            MOTOR_CLOSE_IO;
            stepCounter    = 0;
            pwmModeActive  = 0;
            return;
        }

        if (stepCounter >= PWM_STOP_SPEED && stepCounter <= maxSteps)
        {
            stepCounter -= PWM_STOP_SPEED;

            /* pwmDuty = 1600 - sinTable[step] → ramps 0 up to 1600 */
            pwmDuty = PWM_MAX_VALUE - sinPwmTable[stepCounter];

            if (last_motor_state == PWM_DOWN)
            {
                if (pwmModeActive != PWM_DOWN)
                {
                    TIM_SET_OC3_PWM(TIM_OUTPWM);
                    TIM_SET_OC4_HIGH(TIM_OUTPWM);
                    pwmModeActive = PWM_DOWN;
                }
                TIM_SetCmp3(TIM_OUTPWM, pwmDuty);
            }
            else
            {
                if (pwmModeActive != PWM_UP)
                {
                    TIM_SET_OC4_PWM(TIM_OUTPWM);
                    TIM_SET_OC3_HIGH(TIM_OUTPWM);
                    pwmModeActive = PWM_UP;
                }
                TIM_SetCmp4(TIM_OUTPWM, pwmDuty);
            }
        }

        if (stepCounter < PWM_STOP_SPEED)
        {
            /* ramp complete — fully off */
            MOTOR_CLOSE_IO;
            stepCounter   = 0;
            pwmModeActive = 0;
        }
    }
}


#if 0  /* ==================== ORIGINAL Motor_speed (backup) ====================
 *
 * Original implementation using float sinf() — expensive on Cortex-M0+ (no FPU).
 * Kept for reference / fallback. Remove this block once the lookup-table version
 * is verified on hardware.
 *
void Motor_speed(void)
{
    static uint16_t stepCounter = 0;
    static uint16_t maxSteps = PWM_MAX_COMPARE;

    volatile float angle = (float)stepCounter / (float)maxSteps * 3.1415926f / 2.0f;
    volatile float sinRatio = sinf(angle);
    volatile  uint16_t pwmDuty =1600- (uint16_t)((PWM_MAX_VALUE ) * sinRatio);
    if((Soft_start_moveA == PWM_DOWN || Soft_start_moveA == PWM_UP) && (stepCounter) <= maxSteps)
    {

        if (stepCounter < maxSteps)
        {
            stepCounter  =stepCounter +  PWM_START_SPEED;

						if(stepCounter>maxSteps)
						{
						   stepCounter=maxSteps;
						}
        }
        if (stepCounter >= maxSteps)
        {
						if( Soft_start_moveA == PWM_DOWN )
						{
							PWM_FORCE_CH2_HIGH_CH3_LOW();
						}else
						{
						  PWM_FORCE_CH3_HIGH_CH2_LOW();
						}
        }
      else
        {

						if( Soft_start_moveA == PWM_DOWN )
						{
								TIM_SET_OC3_PWM(TIM_OUTPWM)  ;
								TIM_SetCmp3(TIM_OUTPWM,  (uint16_t)pwmDuty );
								TIM_SET_OC4_HIGH(TIM_OUTPWM);
								last_motor_state=PWM_DOWN;
						}else
						{

								TIM_SET_OC4_PWM(TIM_OUTPWM)  ;
								TIM_SET_OC3_HIGH(TIM_OUTPWM);
								TIM_SetCmp4(TIM_OUTPWM, (uint16_t)pwmDuty );
								last_motor_state=PWM_UP;
						}
        }
    }
			if(Soft_start_moveA == PWM_STOP)
    {

        if( Err.err_bit.Motor_Obs == 1 )
        {
            MOTOR_CLOSE_IO;

            stepCounter = 0;

        }

        if (stepCounter <= maxSteps && stepCounter >= PWM_STOP_SPEED)
        {
									stepCounter -= PWM_STOP_SPEED;
								if(last_motor_state==PWM_DOWN)
							{
											TIM_SET_OC3_PWM(TIM_OUTPWM)  ;
											TIM_SetCmp3(TIM_OUTPWM,   (uint16_t)pwmDuty );
											TIM_SET_OC4_HIGH(TIM_OUTPWM);

							}else
							{
											TIM_SET_OC3_HIGH(TIM_OUTPWM);
											TIM_SET_OC4_PWM(TIM_OUTPWM);
											TIM_SetCmp4(TIM_OUTPWM,   (uint16_t)pwmDuty );
							}
       }


        if (stepCounter < PWM_STOP_SPEED)
        {
							MOTOR_CLOSE_IO;

							stepCounter = 0;

        }

    }

}
   ==================== END ORIGINAL Motor_speed ==================== */
#endif
