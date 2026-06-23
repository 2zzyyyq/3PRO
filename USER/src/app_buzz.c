#include "ALL.h" 
uint8_t CmdBzhx(void);
const tBuzHx Bzx0[] = {{FREQ_NO,    0,  0}, {FREQ_NO,   0,   0}, {FREQ_NO,    0,  0}, {FREQ_NO, 0, 0}};
const tBuzHx Bzx1[] = {{FREQ_1288, 10, 10}, {FREQ_NO,   0,   0}, {FREQ_NO,    0,  0}, {FREQ_NO, 0, 0}}; 
const tBuzHx Bzx2[] = {{FREQ_1288, 40, 30}, {FREQ_NO,   0,   0}, {FREQ_NO,    0,  0}, {FREQ_NO, 0, 0}};
const tBuzHx Bzx3[] = {{FREQ_1238, 12,  6}, {FREQ_1288, 12,  6}, {FREQ_1342, 60, 20}, {FREQ_NO, 0, 0}}; 
const tBuzHx Bzx4[] = {{FREQ_1342, 12,  6}, {FREQ_1288, 12,  6}, {FREQ_1238, 60, 20}, {FREQ_NO, 0, 0}}; 
const tBuzHx Bzx5[] = {{FREQ_1288, 40, 30}, {FREQ_1288, 40, 30}, {FREQ_NO,    0,  0}, {FREQ_NO, 0, 0}}; 
const tBuzHx Bzx6[] = {{FREQ_1288, 40, 30}, {FREQ_1288, 40, 30}, {FREQ_1288, 40, 30}, {FREQ_NO, 0, 0}};

const tBuzHx Bzx7[] = {{FREQ_2300, 12,  6}, {FREQ_2600, 12,  6}, {FREQ_2900, 60, 20}, {FREQ_NO, 0, 0}};
const tBuzHx Bzx8[] = {{FREQ_2900, 12,  6}, {FREQ_2600, 12,  6}, {FREQ_2300, 60, 20}, {FREQ_NO, 0, 0}};

const tBuzHx Bzx9[] = {{FREQ_1288, 30, 10}, {FREQ_1288, 30, 10}, {FREQ_1288, 30, 10}, {FREQ_NULL, 130, 0},
		       {FREQ_1288, 30, 10}, {FREQ_1288, 30, 10}, {FREQ_1288, 30, 10}, {FREQ_NO, 130, 0}}; 
		       
const tBuzHx Bzx10[] = {{FREQ_1288, 30, 10}, {FREQ_1288, 30, 10}, {FREQ_1288, 30, 10}, {FREQ_NULL, 130, 0},
		       {FREQ_1288, 30, 10}, {FREQ_1288, 30, 10}, {FREQ_1288, 30, 10}, {FREQ_NULL, 130, 0},
		       {FREQ_1288, 30, 10}, {FREQ_1288, 30, 10}, {FREQ_1288, 30, 10}, {FREQ_NO, 130, 0}}; 	
uint8_t    g_BuzFrzSta = 0;
uint8_t    g_BuzFrzValue = 0;
uint8_t beep_step=0,beep_step_temp = 0;
tBuzHx  Bzx[12];
tBuzHx *Buz = (tBuzHx *)Bzx; 
void SetupBzhx(uint8_t MBz,uint8_t flag);
					
/***********************************************************************************************************************
* Function Name: SetupBzhx
* Description  : initializes the Buzzer application.
* Arguments    : None
* Return Value : None
***********************************************************************************************************************/ 
void SetupBzhx(uint8_t MBz,uint8_t flag)
{	 	
   uint8_t i;	
	if(!Flag.Beep_open_now)
	{
		BuzVcc_OFF; 
		BUZ_OFF;
		g_BuzFrzValue = 0;
		g_BuzFrzSta = 0;
		beep_step=0;
		Buz = (tBuzHx *)Bzx; 
	if(flag)
	{
		switch(MBz)
		{
		case SHORT_BEEP:
			 for (i = 0; i < 4; i++) Bzx[i] = Bzx1[i];
			 break;
		case LONG_BEEP:
			 for (i = 0; i < 4; i++) Bzx[i] = Bzx2[i]; 
			 break;      
		case RISE_TONE_BEEP:
			 for (i = 0; i < 4; i++) Bzx[i] = Bzx3[i];
			 break; 
		case FALL_TONE_BEEP:
			 for (i = 0; i < 4; i++) Bzx[i] = Bzx4[i]; 
			 break;
		case LONG_BEEP_TWO:
			 for (i = 0; i < 4; i++) Bzx[i] = Bzx5[i];  
			 break;	
		case LONG_BEEP_THREE:
			 for (i = 0; i < 4; i++) Bzx[i] = Bzx6[i];  
			 break;
		case RISE_TONE_BEEP_MODE1:
			 for (i = 0; i < 4; i++) Bzx[i] = Bzx7[i]; 
			 break;	
		case FALL_TONE_BEEP_MODE1:
			 for (i = 0; i < 4; i++) Bzx[i] = Bzx8[i]; 
			 break;		
		default:
			 for (i = 0; i < 4; i++) Bzx[i] = Bzx0[i]; 
			 break;     
		} 
	}
		}	
	}

/***********************************************************************************************************************
* Function Name: CmdBzhx
* Description  : Buzzer application.
* Arguments    : None
* Return Value : None
***********************************************************************************************************************/
uint8_t CmdBzhx(void)
{		
    if (g_BuzFrzValue != Buz->BzFre)
    {
        if (Buz->BzFre == FREQ_NULL)
        {
               BUZ_FRE_OFF;
        }
        else
        {
 						 BUZ_FRE_ON;    
        }
        g_BuzFrzValue = Buz->BzFre;
    }
    
    if (Buz->BzFre != FREQ_NO)
    {
        if (Buz->T_BzV != 0)
        {
            Buz->T_BzV--;
            if (Buz->BzFre != FREQ_NULL)
            {
              BuzVcc_ON;
            }
        }
        else
        {
            BuzVcc_OFF;
        }
        
        if (Buz->T_BzO != 0)
        {
            Buz->T_BzO--;
            if (g_BuzFrzSta == 0)
            {
                g_BuzFrzSta = 1;
                BUZ_ON;
            }
        }
        else
        {
            BUZ_OFF;
            g_BuzFrzSta = 0;
            Buz++;
            beep_step++;
            if (beep_step == beep_step_temp)
            {
                beep_step = 0;
                Flag.Beep_open_now = 0;
            }
        }
    }
    return 0;
}






























































