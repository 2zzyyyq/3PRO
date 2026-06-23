#include "main.h"
void PVD_NVIC_Configuration(void);
void PVD_EXTI_Configuration(void);
void PVD_Init(void);
void WDT_Init(void);
__IO uint16_t LsiFreq = 32000;
void PVD_Init(void)
{
	RCC_EnableAPBPeriphClk(RCC_APB_PERIPH_PWR, ENABLE);
	PVD_EXTI_Configuration();
	PVD_NVIC_Configuration();
	/* Configure the PVD Level to 2.4V ,from 2V ~ 4.8V,one step mean add 200mv*/
	PWR_PVDLevelConfig(PWR_PVD_LEVEL_3V6);
 //	PWR_PVDFilterWidthSet(0x1F);
	//PWR->CTRL &= (~PWR_CTRL_IWDGRSTEN);

	/* Enable the PVD Output */
	PWR_PvdEnable(ENABLE);
 
}
void WDT_Init(void)
{
			    /* Enable PWR Clock */
    RCC_EnableAPBPeriphClk(RCC_APB_PERIPH_PWR, ENABLE);

    /* Check if the system has resumed from IWDG reset */
    if(RCC_GetFlagStatus(RCC_CTRLSTS_FLAG_IWDGRSTF) != RESET)
    {
        /* IWDGRST flag set */
        /* Turn on LED1 */
    
        log_info("reset by IWDG\n");
        /* Clear reset flags */
        RCC_ClearResetFlag();
    }
 

    IWDG_CntReload(LsiFreq / 128);
    /* Reload IWDG counter */
    IWDG_ReloadKey();

    /* Enable IWDG (the LSI oscillator will be enabled by hardware) */
    IWDG_Enable();
 
}



/**
*\*\name    EXTI_Configuration.
*\*\fun     EXTI Configuration.
*\*\param   none
*\*\return  none
**/
void PVD_EXTI_Configuration(void)
{
    EXTI_InitType EXTI_InitStructure;
 
    /* Configure EXTI Line16(PVD Output) to generate an interrupt on rising and
       falling edges */
 
    EXTI_InitStructure.EXTI_Line    = EXTI_LINE7;
    EXTI_InitStructure.EXTI_Mode    = EXTI_Mode_Interrupt;
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising ;
    EXTI_InitStructure.EXTI_LineCmd = ENABLE;
    EXTI_InitPeripheral(&EXTI_InitStructure);
}

/**
*\*\name    NVIC_Configuration.
*\*\fun     NVIC Configuration.
*\*\param   none
*\*\return  none
**/
void PVD_NVIC_Configuration(void)
{
    NVIC_InitType NVIC_InitStructure;

    /* Enable the PVD Interrupt */
    NVIC_InitStructure.NVIC_IRQChannel                   = PVD_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPriority           = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd                = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
}


