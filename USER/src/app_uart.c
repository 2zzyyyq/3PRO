#include "ALL.h" 

#define SET_MODEL_OK 6
#define PACK_SPEC_NUM 18
uint16_t set_light_value;//开灯时灯实时亮度
uint16_t UART_RX_Count = 0,Frame_length=0;
void Judge_Nightlight_Mode(void);
void BootLoardDeal(void);
uint8_t idx=0,offset_value;
uint8_t  Poweron_Set_Model_step=0,g_MiioRoundFlag=0,g_MiioErrorFlag=0;
DeviceStatus_TypeDef g_DevStatus = {0, 0, 0};
uint8_t  _hour,_min;
uint32_t min_data_now,min_start_data_flash,min_stop_data_flash;
const char *Up_Motor_Properties[5]={"2 2 0 0 ","2 2 0 1 ","2 2 0 2 ","2 2 0 3 ","2 2 0 4 "};//电机工作状态
extern      uint16_t UART_RX_Count ; 
extern      uint8_t mcu_common_uart_data_unpack(uint8_t data);
void Model_Net_Change(void);
void Model_Net_Now(void);
#ifdef TESTADC		
const  char MCU_TEST[9]   ="MCU TEST\r";
const  char MCU_TEST_CLOSE[11]     ="MCU NOTEST\r";
#endif

const MiioConst_TypeDef MiioConst = {
    // 电机属性配置
    .Up_Motor_Properties   = {"2 2 0 0 ","2 2 0 1 ","2 2 0 2 ","2 2 0 3 ","2 2 0 4 "},  // 电机A状态字符串
 //   .Up_Motor_Properties_B = {"6 4 0 0 ","6 4 0 1 ","6 4 0 2 ","6 4 0 3 ","6 4 0 4 "},  // 电机B状态字符串

    // Miio 初始化相关
    .SentReast      = "restore\r",                  /* 模块重置 */
    .enter          = "\r\n",                       /* 换行回车 */
    .ModelCheck     = "model\r",                    /* 名称配置查询 */
    .ModelCharCheck = "xiaomi.airer.0003\r",         /* 设备型号字符串 */
    .MCUversion     = "mcu_version 0005\r",         /* 上报MCU版本号 */
    .ModelChar      = "model xiaomi.airer.0003\r",    /* 设备型号完整配置 */
    .ModelPid       = "ble_config set 29375 0005\r", /* 产品识别号PID配置 */
    .MIIOuartarck   = "echo off\r",                 /* 模块回显功能关闭 */

    // MCU 上行指令
    .MCUnet         = "net\r",                      /* 询问网络状态 */
    .DownInstruct   = "get_down\r",                 /* 获取下行指令，间隔150ms */
    .getnettime     = "time\r",                     /* 获取网络时间，间隔30s */
    .error5001      = "error \"bytes limit\" -5001\r", /* 错误：字节长度超限 */
    .error5002      = "error \"error\" -5002\r",       /* 错误：通用错误 */
    .setresult      = "result ",                    /* 按不同下行指令应答前缀 */
    .properties     = "properties_changed",         /* 上报属性，1s间隔 */
    .upevent        = "event_occured",              /* 上报事件，1s间隔 */

    // Miio 状态指令
    .QueryMcuVersion = "down MIIO_mcu_version_req\r", /* 版本查询 */
    .Arcknone        = "down none\r",                  /* 无命令下发 */
    .ArckOk          = "ok\r",                         /* MIIO接收完成应答 */
    
    .offline         = "offline\r",                   /* WiFi连接中或掉线 */
    .local           = "local\r",                     /* WiFi连上路由器 */
    .cloud           = "cloud\r",                     /* WiFi连接上米家平台 */
    .uap             = "uap\r",                       /* UAP模式等待快连 */
    .unprov          = "unprov\r",                    /* WiFi从米家平台解绑 */
    .updating        = "updating\r",                  /* 固件升级中 */
    
    .UnknownCommand  = "down unknown_command\r",      /* 未知无用命令 */
    .WIFIoffline     = "down MIIO_net_change offline\r",   /* WiFi状态：连接中 */
    .WIFIlocal       = "down MIIO_net_change local\r",     /* WiFi状态：已连路由器 */
    .WIFIcloud       = "down MIIO_net_change cloud\r",     /* WiFi状态：已连米家云 */
    .WIFIuap         = "down MIIO_net_change uap\r",       /* WiFi状态：快连模式 */
    .WIFIunprov      = "down MIIO_net_change unprov\r",    /* WiFi状态：已解绑 */
    .WIFIupdating    = "down MIIO_net_change updating\r",  /* WiFi状态：升级中 */
    
    .error           = "error\r",                     /* 通用错误应答 */
    .poweroff        = "down power_off\r",            /* 关机指令 */
    
    .SetProperties   = "down set_properties\r",       /* 米家设置属性指令 */
    .ActionImplement = "down action\r",               /* 动作执行指令 */
    .QueryOneState   = "down get_properties\r",       /* 查询单个属性状态 */
    .bootloard       = "down update_fw\r",            /* 进入固件升级模式 */

    // 自动升级相关
    .auto_on     = "set_mcu_auto_ota on\r",       /* 打开自动更新 */
    .auto_off    = "set_mcu_auto_ota off\r",      /* 关闭自动更新 */
    .autoupdate  = "down update_fw auto\r",       /* 自动更新指令 */
    .forceupdate = "down update_fw force\r",      /* 强制自动更新 */
    .autoready   = "result \"ready\"\r",          /* 准备更新应答 */
    .autobusy    = "result \"busy\"\r"            /* 不适合更新应答 */
};


void r_uart0_service(void)
{ 
  if(get_queue_total_data() > 0)
  {
    if(mcu_common_uart_data_unpack(Queue_Read_Byte()))//收到一包数据
    {
	        data_handle();//解析数据包
            memset((char *)uart1_rx_buf,0,sizeof(uart1_rx_buf));
	        UART_RX_Count = 0;
			 
    }
  }
 }


/***********************************************************************************************************************
* Function Name: XMModelSet
* Description  : Miio Init
* Arguments    : None
* Return Value : None
***********************************************************************************************************************/
void PowerOn_ModelSet(void)
{	
	
	switch (g_DevStatus.Poweron_Set_Model_step)
	{

	case 0:
		 memcpy((uint8_t *)uart1_tx_buf, MiioConst.ModelChar, strlen (MiioConst.ModelChar));
		 r_uart0_send_bytes((uint8_t *)uart1_tx_buf, strlen (MiioConst.ModelChar)); //上报名称	

		 break;
	case 1:
		 memcpy((uint8_t *)uart1_tx_buf, MiioConst.MCUversion, strlen (MiioConst.MCUversion));
		 r_uart0_send_bytes((uint8_t *)uart1_tx_buf, strlen (MiioConst.MCUversion));//上报版本		 
		 break;
	case 2:	
		 memcpy((uint8_t *)uart1_tx_buf, MiioConst.MIIOuartarck, strlen (MiioConst.MIIOuartarck));//模块通信检测
		 r_uart0_send_bytes((uint8_t *)uart1_tx_buf, strlen (MiioConst.MIIOuartarck));			 		
		 break;
	case 3:	
		 memcpy((uint8_t *)uart1_tx_buf, MiioConst.ModelPid, strlen(MiioConst.ModelPid)); //产品识别号
		 r_uart0_send_bytes((uint8_t *)uart1_tx_buf, strlen (MiioConst.ModelPid));		  
        	 break;
	case 4:
		 memcpy((uint8_t *)uart1_tx_buf, MiioConst.MCUversion, strlen (MiioConst.MCUversion));
        	 r_uart0_send_bytes((uint8_t *)uart1_tx_buf, strlen (MiioConst.MCUversion));//上报版本			 	
		 break;	
	case 5:
		 memcpy((uint8_t *)uart1_tx_buf, MiioConst.auto_on, strlen (MiioConst.auto_on));//设置模组自动升级程序
       		 r_uart0_send_bytes((uint8_t *)uart1_tx_buf, strlen (MiioConst.auto_on));
		 break;	
	default:     break;
	}
}
/*****************************************************************************
 * data_handle — MIIO protocol command dispatcher
 *
 * Called from r_uart0_service() when a complete CRLF-terminated frame arrives.
 * Routes commands in 3 phases:
 *   Phase 1: PCBA self-test     (Enter_Pcba_test_flag)
 *   Phase 2: MIIO pairing       (step < SET_MODEL_OK)
 *   Phase 3: Normal operation   (step >= SET_MODEL_OK)
 *
 * Optimization notes:
 *   - First-byte pre-filter (buf[0]) avoids unnecessary strcmp scans
 *   - Early returns prevent fall-through after command handled
 *   - uart1_tx_buf used for response assembly (not rx_buf)
 *****************************************************************************/
void data_handle(void)
{
    char *buf = (char *)uart1_rx_buf;
    char  first = buf[0];
    uint8_t step = g_DevStatus.Poweron_Set_Model_step;

    /* ====== Phase 1: PCBA self-test ====== */
    if (Flag.Enter_Pcba_test_flag)
    {
        if (first == 'o' && strcmp(MiioConst.ArckOk, buf) == 0)
        {
            // Pcba_test.wifi_test_flag = 1;
        }
        /* self-test mode: skip normal processing */
        goto clear_buf;
    }

    /* ====== Phase 2: MIIO pairing handshake ====== */
    if (step < SET_MODEL_OK)
    {
        if (first == 'o' && strcmp(MiioConst.ArckOk, buf) == 0)
        {
            /* step==4 → skip to 6 (model query passed); else → +1 */
            step += (step == 4) ? 2 : 1;
            g_DevStatus.Poweron_Set_Model_step = step;
        }
        else if (first == 'e' && strcmp(MiioConst.error, buf) == 0)
        {
            /* step==4 → allow error as valid response for model query */
            if (step == 4)
            {
                g_DevStatus.Poweron_Set_Model_step = step + 1;
            }
        }
        goto clear_buf;
    }

    /* ====== Phase 3: Normal operation (paired) ====== */

    /* --- UART frame overflow error --- */
    if (Flag.Uart1_Err)
    {
        Flag.Uart1_Err = 0;
        r_uart0_send_bytes((uint8_t *)MiioConst.error5001, strlen(MiioConst.error5001));
        goto clear_buf;
    }

    /* --- Idle heartbeat: "ok\r" or "down none\r" → ignore --- */
    if ((first == 'o' && strcmp(MiioConst.ArckOk, buf) == 0)
     || (first == 'd' && strcmp(MiioConst.Arcknone, buf) == 0))
    {
        goto clear_buf;
    }

    /* --- Dispatch by first byte --- */
    switch (first)
    {
    case 'd':  /* "down ..." commands */
    {
        /* skip past "down " prefix (5 chars) for sub-matching */
        char *payload = buf + 5;

        /* -- OTA: "down update_fw" (exact) -- */
        if (strcmp(MiioConst.bootloard, buf) == 0)
        {
            Flash_Data.Quite_updata = 1;
            BootLoardDeal();
            return; /* BootLoardDeal may reset; if not, return */
        }

        /* -- OTA: "down update_fw auto" / "down update_fw force" (prefix 19) -- */
        if (strncmp(MiioConst.autoupdate, buf, 19) == 0)
        {
            Flash_Data.Quite_updata = 0;
            /* BootLoardDeal() intentionally disabled for silent OTA */
            goto clear_buf;
        }

        /* -- Version query: "down MIIO_mcu_version_req" (prefix 18) -- */
        if (strncmp(MiioConst.QueryMcuVersion, buf, 18) == 0)
        {
            r_uart0_send_bytes((uint8_t *)MiioConst.MCUversion,
                               strlen(MiioConst.MCUversion));
            goto clear_buf;
        }

        /* -- Properties query: "down get_properties" (prefix 18) -- */
        if (strncmp(MiioConst.QueryOneState, buf, 18) == 0)
        {
            Return_Properties();
            goto clear_buf;
        }

        /* -- Properties set: "down set_properties" (prefix 18) -- */
        if (strncmp(MiioConst.SetProperties, buf, 18) == 0)
        {
            Set_Properties();
            goto clear_buf;
        }

        /* -- Action: "down action" (prefix 11) -- */
        if (strncmp(MiioConst.ActionImplement, buf, 11) == 0)
        {
            Action_deal();
            goto clear_buf;
        }

        /* -- Network state change (long format): "down MIIO_net_change xxx" -- */
        /* (Model_Net_Change handler currently disabled) */
 	
	//Model_Net_Now();
        /* -- Power off: "down power_off" -- */
        if (strcmp(MiioConst.poweroff, buf) == 0)
        {
            if (!g_DevStatus.g_MiioErrorFlag)
            {
                g_DevStatus.g_MiioErrorFlag = 1;
                r_uart0_send_bytes((uint8_t *)MiioConst.error5002,
                                   strlen(MiioConst.error5002));
            }
            goto clear_buf;
        }

        /* -- Unknown command: "down unknown_command" -- */
        if (strcmp(MiioConst.UnknownCommand, buf) == 0)
        {
            if (!g_DevStatus.g_MiioErrorFlag)
            {
                g_DevStatus.g_MiioErrorFlag = 1;
                r_uart0_send_bytes((uint8_t *)MiioConst.error5002,
                                   strlen(MiioConst.error5002));
            }
            goto clear_buf;
        }
    }
    break;


    case 'e':  /* "error\r" */
        if (strcmp(MiioConst.error, buf) == 0)
        {
            if (!g_DevStatus.g_MiioErrorFlag)
            {
                g_DevStatus.g_MiioErrorFlag = 1;
                r_uart0_send_bytes((uint8_t *)MiioConst.error5002,
                                   strlen(MiioConst.error5002));
                r_uart0_protocol_init(); /* flush queue */
            }
            goto clear_buf;
        }
        break;

    case 'l':  /* "local\r" — network state, handler disabled */
    case 'c':  /* "cloud\r" — network state, handler disabled */
    case 'u':  /* "uap\r" / "unprov\r" / "updating\r" */
	           		Model_Net_Change();
		
        break;

#ifdef TESTADC
    case 'M':  /* "MCU TEST\r" / "MCU NOTEST\r" */
        if (strncmp(MCU_TEST, buf, sizeof(MCU_TEST) - 1) == 0)
        {
            Flag.mcu_wifi_mode = 1;
        }
        else if (strncmp(MCU_TEST_CLOSE, buf, sizeof(MCU_TEST_CLOSE) - 1) == 0)
        {
            Flag.mcu_wifi_mode = 0;
        }
        /* Force pairing complete so normal 150ms loop runs */
        g_DevStatus.Poweron_Set_Model_step = SET_MODEL_OK;
        break;
#endif

    default:    r_uart0_send_bytes((uint8_t *)MiioConst.error5002,
                                   strlen(MiioConst.error5002));
                  break;
    }

clear_buf:
    memset(uart1_rx_buf, 0, Frame_length > 180 ? Frame_length : 180);
}

void BootLoardDeal(void)
{ 
 	MOTOR_DN_IO(0);//停止电机
	MOTOR_UP_IO(0);
 
	BuzVcc_OFF;
	BUZ_OFF;
	Flash_Data.Device_updata=2;//保存升级状态
	Flash_Data.Device_light_state=Device_State_Data.Light_State_Byte;//保存照明状态
	Flash_Data.Light_pwm_value=Device_State_Data.Light_Bright;//保存亮度百分比
	
	Flash_Data.Device_motor_state=Device_State_Data.Motor_State_Byte;//保存电机状态
	UART_ConfigInt(LOG_UARTx, UART_INT_RXDNE, DISABLE);
 
	 __disable_irq();
	
	LED_LIGHT_OFF;
	FDL_Block0_Save_Times=FDL_ONCE_MAX_WRITE_NUM-1;	
	Flash_WriteStruct(FDL_Block0+FDL_Block0_Save_Times*sizeof(Flash_Data),(uint8_t*)&Flash_Data,sizeof(Flash_Data));//写入一次	
	FDL_Block1_Save_Times=FDL_BLOCK1_ONCE_MAX_WRITE_NUM-1;
	Flash_WriteStruct(FDL_Block1+FDL_Block1_Save_Times*sizeof(Ell_Data),(uint8_t*)&Ell_Data,sizeof(Ell_Data));//块1写入一次	
    NVIC_SystemReset();
	while (1) 
	{
		
	}
} 
void _150ms_work(void)
{	
	uint16_t  _data;
	if(g_DevStatus.Poweron_Set_Model_step>=SET_MODEL_OK)
	{
		if(g_MiioRoundFlag==1)//
		{
			g_MiioRoundFlag=0;
			memcpy((uint8_t *)uart1_tx_buf, MiioConst.DownInstruct, strlen(MiioConst.DownInstruct));//获取下行指令
			r_uart0_send_bytes((uint8_t *)uart1_tx_buf, strlen(MiioConst.DownInstruct));
		}
		else
		{
			Time._150ms_Count++;
			if(Time._150ms_Count%3==0)//更新一次位置信息
			{
				if((Ell_Data.Motor_Current_Position>=DOWN_POSITION_VALUE)||(Flag.Down_limit))
				{
					Device_State_Data.Motor_Position=100;
				}
				else
				{
					if(Ell_Data.Motor_Current_Position<UP_POSITION_VALUE)
					{
						Device_State_Data.Motor_Position=0;
					}
					else
					{
						_data=(Ell_Data.Motor_Current_Position-UP_POSITION_VALUE)*1000.0/STROKE_TOTAL;//百分比先放大10倍
						//四舍五入
						if(_data%10>=5)
						{
							Device_State_Data.Motor_Position=_data/10+1;
						} 
						else
						{
							Device_State_Data.Motor_Position=_data/10;
						}
					}
				}
			}
			if(Time._150ms_Count%25==0)//5s，读取一次模块联网状态
			{				
				memcpy((uint8_t *)uart1_tx_buf, MiioConst.MCUnet, strlen(MiioConst.MCUnet));
				r_uart0_send_bytes((uint8_t *)uart1_tx_buf, strlen(MiioConst.MCUnet)); 
				g_MiioRoundFlag=1;	
			}
			else if(Time._150ms_Count==157)//30s,获取一次网络时间
			{
				Time._150ms_Count=0;
				if(Net_state==NET_CLOUD)
				{			
					memcpy((uint8_t *)uart1_tx_buf, MiioConst.getnettime, strlen(MiioConst.getnettime));
					r_uart0_send_bytes((uint8_t *)uart1_tx_buf, strlen(MiioConst.getnettime));  
					g_MiioRoundFlag=1;	
				}
			}
			else
			{	
				if(properties_up())//上报属性
				{
					g_MiioRoundFlag=1;
				}
				else
				{
					memcpy((uint8_t *)uart1_tx_buf, MiioConst.DownInstruct, strlen(MiioConst.DownInstruct));//获取下行指令
					r_uart0_send_bytes((uint8_t *)uart1_tx_buf, strlen(MiioConst.DownInstruct));
				}	
			}
		}
	}
	else
	{
		Time._150ms_Count++;
		if(Time._150ms_Count>=3)//更新一次位置信息
		{
			Time._150ms_Count=0;
			if((Ell_Data.Motor_Current_Position>=DOWN_POSITION_VALUE)||(Flag.Down_limit))
			{
				Device_State_Data.Motor_Position=100;
			}
			else
			{
				if(Ell_Data.Motor_Current_Position<UP_POSITION_VALUE)
				{
					Device_State_Data.Motor_Position=0;
				}
				else
				{
					_data=(Ell_Data.Motor_Current_Position-UP_POSITION_VALUE)*1000.0/STROKE_TOTAL;//百分比先放大10倍
					//四舍五入
					if(_data%10>=5)
					{
						Device_State_Data.Motor_Position=_data/10+1;
					} 
					else
					{
						Device_State_Data.Motor_Position=_data/10;
					}
				}
			}
		}	
	}	
}
uint8_t properties_up(void)//设备状态上报
{
	uint8_t _data[4];
	uint8_t ret = FALSE;
	char *UpLoad_Buf;
	uint8_t BufNum;
	uint8_t Property_num=0;
	if(Net_state!=NET_CLOUD)//
	{
		//return ret;
	}
	BufNum=18;
	memcpy((uint8_t *)uart1_tx_buf, MiioConst.properties, 18);
	//A,B故障属性
	if((Device_State_Data.Err_Byte&0x0f)!=(Up_State_Data.Err_Byte&0x0f))//
	{
		UpLoad_Buf = " 2 1 0";
		memcpy((uint8_t *)uart1_tx_buf + BufNum, UpLoad_Buf, 6);
		BufNum += 6;

		 Up_State_Data.Err_Byte=Device_State_Data.Err_Byte;
		ret = TRUE;
		Property_num++;
		if(Property_num==PACK_SPEC_NUM)
		{
			goto send_data;
		}
	}
 
	//A故障属性
	if((Device_State_Data.Err_Byte&0x0f)!=(Up_State_Data.Err_Byte&0x0f))//
	{
		UpLoad_Buf = " 2 1 0";
		memcpy((uint8_t *)uart1_tx_buf + BufNum, UpLoad_Buf, 6);
		BufNum += 6;
		if((Err.data&0x01)||(Err.data&0x08))//有遇阻
		{
			uart1_tx_buf[BufNum-1]=0x31;
		}
		else if(Err.data&0x02)//有超重
		{
			uart1_tx_buf[BufNum-1]=0x32;
		}
		else if(Err.data&0x04)//有电机堵转
		{
			uart1_tx_buf[BufNum-1]=0x33;
		}
		else
		{
			uart1_tx_buf[BufNum-1]=0x30;
		}
		Up_State_Data.Err_Byte=Device_State_Data.Err_Byte;
		ret = TRUE;
		Property_num++;
		if(Property_num==PACK_SPEC_NUM)
		{
			goto send_data;
		}
	}

	//A晾杆状态
	if(Device_State_Data.Motor_State_Byte!=Up_State_Data.Motor_State_Byte)//
	{
		UpLoad_Buf = " 2 4 0";	
		memcpy((uint8_t *)uart1_tx_buf + BufNum, UpLoad_Buf, 6);
		BufNum += 6;		
		uart1_tx_buf[BufNum - 1] = Device_State_Data.Motor_State_Byte+0x30;		
		
		Up_State_Data.Motor_State_Byte=Device_State_Data.Motor_State_Byte;	
		ret = TRUE;
		Property_num++;
		if(Property_num==PACK_SPEC_NUM)
		{
			goto send_data;
		}
	}

	//A晾杆位置	
	if(Device_State_Data.Motor_Position!=Up_State_Data.Motor_Position)//
	{
		UpLoad_Buf = " 2 11 0";
		if(Device_State_Data.Motor_Position<10)
		{
			memcpy((uint8_t *)uart1_tx_buf + BufNum, UpLoad_Buf, 7);
			BufNum += 7;
			uart1_tx_buf[BufNum - 1] =Device_State_Data.Motor_Position+0x30;
		}
		else if(Device_State_Data.Motor_Position<100)
		{	
			_data[0]=Device_State_Data.Motor_Position/10;
			_data[1]=Device_State_Data.Motor_Position%10;
			
			memcpy((uint8_t *)uart1_tx_buf + BufNum, UpLoad_Buf, 7);
			BufNum += 8;
			uart1_tx_buf[BufNum - 2] =_data[0]+0x30;
			uart1_tx_buf[BufNum - 1] =_data[1]+0x30;
		}
		else
		{
			memcpy((uint8_t *)uart1_tx_buf + BufNum, UpLoad_Buf, 7);
			BufNum += 9;
			uart1_tx_buf[BufNum - 3] =0x31;
			uart1_tx_buf[BufNum - 2] =0x30;
			uart1_tx_buf[BufNum - 1] =0x30;
		}		
		Up_State_Data.Motor_Position=Device_State_Data.Motor_Position;
		ret = TRUE;
		Property_num++;
		if(Property_num==PACK_SPEC_NUM)
		{
			goto send_data;
		}
	}

	//设定A运行位置上报
	if(Device_State_Data.set_position!=Up_State_Data.set_position)//
	{
		UpLoad_Buf = " 2 12 0";
		if(Device_State_Data.set_position<10)
		{
			memcpy((uint8_t *)uart1_tx_buf + BufNum, UpLoad_Buf, 7);
			BufNum += 7;
			uart1_tx_buf[BufNum - 1] =Device_State_Data.set_position+0x30;
		}
		else if(Device_State_Data.set_position<100)
		{	
			_data[0]=Device_State_Data.set_position/10;
			_data[1]=Device_State_Data.set_position%10;
			
			memcpy((uint8_t *)uart1_tx_buf + BufNum, UpLoad_Buf, 7);
			BufNum += 8;
			uart1_tx_buf[BufNum - 2] =_data[0]+0x30;
			uart1_tx_buf[BufNum - 1] =_data[1]+0x30;
		}
		else
		{
			memcpy((uint8_t *)uart1_tx_buf + BufNum, UpLoad_Buf, 8);
			BufNum += 9;
			uart1_tx_buf[BufNum - 3] =0x31;
			uart1_tx_buf[BufNum - 2] =0x30;
			uart1_tx_buf[BufNum - 1] =0x30;
		}	
	
		Up_State_Data.set_position=Device_State_Data.set_position;
		ret = TRUE;
		Property_num++;
		if(Property_num==PACK_SPEC_NUM)
		{
			goto send_data;
		}			
	}

	//A杆电子上限位
	if(Device_State_Data.electronic_up_limit!=Up_State_Data.electronic_up_limit)
	{
		UpLoad_Buf = " 2 13 0";;
		if(Device_State_Data.electronic_up_limit<10)
		{
			memcpy((uint8_t *)uart1_tx_buf + BufNum, UpLoad_Buf, 7);
			BufNum += 7;
			uart1_tx_buf[BufNum - 1] =Device_State_Data.electronic_up_limit+0x30;
		}
		else if(Device_State_Data.electronic_up_limit<100)
		{	
			_data[0]=Device_State_Data.electronic_up_limit/10;
			_data[1]=Device_State_Data.electronic_up_limit%10;
			
			memcpy((uint8_t *)uart1_tx_buf + BufNum, UpLoad_Buf, 7);
			BufNum += 8;
			uart1_tx_buf[BufNum - 2] =_data[0]+0x30;
			uart1_tx_buf[BufNum - 1] =_data[1]+0x30;
		}
		else
		{
			memcpy((uint8_t *)uart1_tx_buf + BufNum, UpLoad_Buf, 7);
			BufNum += 9;
			uart1_tx_buf[BufNum - 3] =0x31;
			uart1_tx_buf[BufNum - 2] =0x30;
			uart1_tx_buf[BufNum - 1] =0x30;
		}
		Up_State_Data.electronic_up_limit=Device_State_Data.electronic_up_limit;
		ret = TRUE;
		Property_num++;
		if(Property_num==PACK_SPEC_NUM)
		{
			goto send_data;
		}
	}

	//A杆电子下限位
	if(Device_State_Data.electronic_down_limit!=Up_State_Data.electronic_down_limit)
	{
		UpLoad_Buf = " 2 14 0";
		if(Device_State_Data.electronic_down_limit<10)
		{
			memcpy((uint8_t *)uart1_tx_buf + BufNum, UpLoad_Buf, 7);
			BufNum += 7;
			uart1_tx_buf[BufNum - 1] =Device_State_Data.electronic_down_limit+0x30;
		}
		else if(Device_State_Data.electronic_down_limit<100)
		{	
			_data[0]=Device_State_Data.electronic_down_limit/10;
			_data[1]=Device_State_Data.electronic_down_limit%10;
			
			memcpy((uint8_t *)uart1_tx_buf + BufNum, UpLoad_Buf, 7);
			BufNum += 8;
			uart1_tx_buf[BufNum - 2] =_data[0]+0x30;
			uart1_tx_buf[BufNum - 1] =_data[1]+0x30;
		}
		else
		{
			memcpy((uint8_t *)uart1_tx_buf + BufNum, UpLoad_Buf, 7);
			BufNum += 9;
			uart1_tx_buf[BufNum - 3] =0x31;
			uart1_tx_buf[BufNum - 2] =0x30;
			uart1_tx_buf[BufNum - 1] =0x30;
		}
		Up_State_Data.electronic_down_limit=Device_State_Data.electronic_down_limit;
		ret = TRUE;	
		Property_num++;
		if(Property_num==PACK_SPEC_NUM)
		{
			goto send_data;
		}
	}

	//A杆收合开关状态
	if(Device_State_Data.convergent!=Up_State_Data.convergent)
	{
		if(Device_State_Data.convergent==CONVERGENT_OFF)
		{
			UpLoad_Buf=" 2 15 false";//
			memcpy((uint8_t *)uart1_tx_buf + BufNum, UpLoad_Buf, 11);
			BufNum += 11;		
		}
		else if(Device_State_Data.convergent==CONVERGENT_ON)
		{
			UpLoad_Buf=" 2 15 true";
			memcpy((uint8_t *)uart1_tx_buf + BufNum, UpLoad_Buf, 10);
			BufNum += 10;			
		}	
		Up_State_Data.convergent=Device_State_Data.convergent;
		ret = TRUE;
		Property_num++;
		if(Property_num==PACK_SPEC_NUM)
		{
			goto send_data;
		}
	}

	//当前A速度
	if(Device_State_Data.current_speed!=Up_State_Data.current_speed)
	{
		UpLoad_Buf =  " 2 16 0";
		if(Device_State_Data.current_speed<10)
		{
			memcpy((uint8_t *)uart1_tx_buf + BufNum, UpLoad_Buf, 7);
			BufNum += 9;
			uart1_tx_buf[BufNum - 3] =0x30;
			uart1_tx_buf[BufNum - 2] ='.';
			uart1_tx_buf[BufNum - 1] =Device_State_Data.current_speed+0x30;
		}
		else if(Device_State_Data.current_speed<100)
		{	
			_data[0]=Device_State_Data.current_speed/10;
			_data[1]=Device_State_Data.current_speed%10;
			
			memcpy((uint8_t *)uart1_tx_buf + BufNum, UpLoad_Buf, 7);
			BufNum += 9;
			uart1_tx_buf[BufNum - 3] =_data[0]+0x30;
			uart1_tx_buf[BufNum - 2] ='.';
			uart1_tx_buf[BufNum - 1] =_data[1]+0x30;
		}
		else
		{
			_data[0]=Device_State_Data.current_speed/100;
			_data[1]=Device_State_Data.current_speed%100/10;
			_data[2]=Device_State_Data.current_speed%10;
			
			memcpy((uint8_t *)uart1_tx_buf + BufNum, UpLoad_Buf, 7);
			BufNum += 10;
			uart1_tx_buf[BufNum - 4] =_data[0]+0x30;
			uart1_tx_buf[BufNum - 3] =_data[1]+0x30;
			uart1_tx_buf[BufNum - 2] ='.';
			uart1_tx_buf[BufNum - 1] =_data[2]+0x30;
		}
		Up_State_Data.current_speed=Device_State_Data.current_speed;
		ret = TRUE;
		Property_num++;
		if(Property_num==PACK_SPEC_NUM)
		{
			goto send_data;
		}
	}

	//照明状态
	if(Device_State_Data.Light_State_Byte!=Up_State_Data.Light_State_Byte)//
	{
		if(Device_State_Data.Light_State_Byte==LIGHT_OFF)
		{
			UpLoad_Buf=" 3 1 false";//
			memcpy((uint8_t *)uart1_tx_buf + BufNum, UpLoad_Buf, 10);
			BufNum += 10;		
		}
		else if(Device_State_Data.Light_State_Byte==LIGHT_ON)
		{
			UpLoad_Buf=" 3 1 true";//
			memcpy((uint8_t *)uart1_tx_buf + BufNum, UpLoad_Buf, 9);
			BufNum += 9;			
		}	
		Up_State_Data.Light_State_Byte=Device_State_Data.Light_State_Byte;
		ret = TRUE;
		Property_num++;
		if(Property_num==PACK_SPEC_NUM)
		{
			goto send_data;
		}
	}
	//照明亮度
	if(Device_State_Data.Light_Bright!=Up_State_Data.Light_Bright)//
	{
		UpLoad_Buf =" 3 2 0";
		if(Device_State_Data.Light_Bright<10)
		{
			memcpy((uint8_t *)uart1_tx_buf + BufNum, UpLoad_Buf, 6);
			BufNum += 6;
			uart1_tx_buf[BufNum - 1] =Device_State_Data.Light_Bright+0x30;
		}
		else if(Device_State_Data.Light_Bright<100)
		{	
			_data[0]=Device_State_Data.Light_Bright/10;
			_data[1]=Device_State_Data.Light_Bright%10;
			
			memcpy((uint8_t *)uart1_tx_buf + BufNum, UpLoad_Buf, 6);
			BufNum += 7;
			uart1_tx_buf[BufNum - 2] =_data[0]+0x30;
			uart1_tx_buf[BufNum - 1] =_data[1]+0x30;
		}
		else
		{
			memcpy((uint8_t *)uart1_tx_buf + BufNum, UpLoad_Buf, 6);
			BufNum += 8;
			uart1_tx_buf[BufNum - 3] =0x31;
			uart1_tx_buf[BufNum - 2] =0x30;
			uart1_tx_buf[BufNum - 1] =0x30;
		}	
		Up_State_Data.Light_Bright=Device_State_Data.Light_Bright;
		ret = TRUE;
		Property_num++;
		if(Property_num==PACK_SPEC_NUM)
		{
			goto send_data;
		}
	}
	//照明色温
	if(Device_State_Data.Light_colour_temperature!=Up_State_Data.Light_colour_temperature)//
	{
		UpLoad_Buf = " 3 3 0";
		if(Device_State_Data.Light_colour_temperature<10)
		{
			memcpy((uint8_t *)uart1_tx_buf + BufNum, UpLoad_Buf, 6);
			BufNum += 6;
			uart1_tx_buf[BufNum - 1] =Device_State_Data.Light_colour_temperature+0x30;
		}
		else if(Device_State_Data.Light_colour_temperature<100)
		{	
			_data[0]=Device_State_Data.Light_colour_temperature/10;
			_data[1]=Device_State_Data.Light_colour_temperature%10;
			
			memcpy((uint8_t *)uart1_tx_buf + BufNum, UpLoad_Buf, 6);
			BufNum += 7;
			uart1_tx_buf[BufNum - 2] =_data[0]+0x30;
			uart1_tx_buf[BufNum - 1] =_data[1]+0x30;
		}
		else if(Device_State_Data.Light_colour_temperature<1000)
		{
			_data[0]=Device_State_Data.Light_colour_temperature/100;
			_data[1]=Device_State_Data.Light_colour_temperature%100/10;
			_data[2]=Device_State_Data.Light_colour_temperature%10;
			memcpy((uint8_t *)uart1_tx_buf + BufNum, UpLoad_Buf, 6);
			BufNum += 8;
			uart1_tx_buf[BufNum - 3] =_data[0]+0x30;
			uart1_tx_buf[BufNum - 2] =_data[1]+0x30;
			uart1_tx_buf[BufNum - 1] =_data[2]+0x30;
		}	
		else if(Device_State_Data.Light_colour_temperature<10000)
		{
			_data[0]=Device_State_Data.Light_colour_temperature/1000;
			_data[1]=Device_State_Data.Light_colour_temperature%1000/100;	
			_data[2]=Device_State_Data.Light_colour_temperature%100/10;	
			_data[3]=Device_State_Data.Light_colour_temperature%10;
			memcpy((uint8_t *)uart1_tx_buf + BufNum, UpLoad_Buf, 6);
			BufNum += 9;
			uart1_tx_buf[BufNum - 4] =0x30+_data[0];
			uart1_tx_buf[BufNum - 3] =0x30+_data[1];
			uart1_tx_buf[BufNum - 2] =0x30+_data[2];
			uart1_tx_buf[BufNum - 1] =0x30+_data[3];
		}
		Up_State_Data.Light_colour_temperature=Device_State_Data.Light_colour_temperature;
		ret = TRUE;
	}

	//夜灯开关
	if(Device_State_Data.nightlight_switch!=Up_State_Data.nightlight_switch)//
	{
		if(Device_State_Data.nightlight_switch==LIGHT_OFF)
		{
			UpLoad_Buf=" 3 5 false";//
			memcpy((uint8_t *)uart1_tx_buf + BufNum, UpLoad_Buf, 10);
			BufNum += 10;		
		}
		else if(Device_State_Data.nightlight_switch==LIGHT_ON)
		{
			UpLoad_Buf=" 3 5 true";//
			memcpy((uint8_t *)uart1_tx_buf + BufNum, UpLoad_Buf, 9);
			BufNum += 9;			
		}	
		Up_State_Data.nightlight_switch=Device_State_Data.nightlight_switch;
		ret = TRUE;
		Property_num++;
		if(Property_num==PACK_SPEC_NUM)
		{
			goto send_data;
		}
	}	
	//夜灯亮度
	if(Device_State_Data.brightness!=Up_State_Data.brightness)//
	{
		UpLoad_Buf =" 3 6 0";
		if(Device_State_Data.brightness<10)
		{
			memcpy((uint8_t *)uart1_tx_buf + BufNum, UpLoad_Buf, 6);
			BufNum += 6;
			uart1_tx_buf[BufNum - 1] =Device_State_Data.brightness+0x30;
		}
		else if(Device_State_Data.brightness<100)
		{	
			_data[0]=Device_State_Data.brightness/10;
			_data[1]=Device_State_Data.brightness%10;
			
			memcpy((uint8_t *)uart1_tx_buf + BufNum, UpLoad_Buf, 6);
			BufNum += 7;
			uart1_tx_buf[BufNum - 2] =_data[0]+0x30;
			uart1_tx_buf[BufNum - 1] =_data[1]+0x30;
		}
		else
		{
			memcpy((uint8_t *)uart1_tx_buf + BufNum, UpLoad_Buf, 6);
			BufNum += 8;
			uart1_tx_buf[BufNum - 3] =0x31;
			uart1_tx_buf[BufNum - 2] =0x30;
			uart1_tx_buf[BufNum - 1] =0x30;
		}	
		Up_State_Data.brightness=Device_State_Data.brightness;
		ret = TRUE;
		Property_num++;
		if(Property_num==PACK_SPEC_NUM)
		{
			goto send_data;
		}
	}
	//提示音开关
	if(Device_State_Data.Beep_Enable!=Up_State_Data.Beep_Enable)//
	{	
		if(Device_State_Data.Beep_Enable==BEEP_OFF)
		{
			UpLoad_Buf=" 5 1 false";//
			memcpy((uint8_t *)uart1_tx_buf + BufNum, UpLoad_Buf, 10);
			BufNum += 10;		
		}
		else if(Device_State_Data.Beep_Enable==BEEP_ON)
		{
			UpLoad_Buf=" 5 1 true";//
			memcpy((uint8_t *)uart1_tx_buf + BufNum, UpLoad_Buf, 9);
			BufNum += 9;			
		}
		Up_State_Data.Beep_Enable=Device_State_Data.Beep_Enable;
		ret = TRUE;		
		 Property_num++;
		if(Property_num==PACK_SPEC_NUM)
		{
			goto send_data;
		}
	}
	if(Device_State_Data.Control_open!=Up_State_Data.Control_open)//
	{
 
		if(Device_State_Data.Control_open==0)
		{
			UpLoad_Buf=" 8 1 false";//
			memcpy((uint8_t *)uart1_tx_buf + BufNum, UpLoad_Buf, 10);
			BufNum += 10;		
		}
		else if(Device_State_Data.Control_open==1)
		{
			UpLoad_Buf=" 8 1 true";//
			memcpy((uint8_t *)uart1_tx_buf + BufNum, UpLoad_Buf, 9);
			BufNum += 9;			
		}
		Up_State_Data.Control_open=Device_State_Data.Control_open;
		ret = TRUE;		
	}
	if(Device_State_Data.Control_num!=Up_State_Data.Control_num)
	{
				char UpLoadBuf1[90];
				memset(UpLoadBuf1, 0, sizeof(UpLoadBuf1));
				switch(Flash_Data.Remote_Control_Num)	
				{
				case 0: 
					snprintf(UpLoadBuf1,
					sizeof(UpLoadBuf1)-1,
					" 8 3 \"%02X:%02X:%02X:%02X\"",
					(0xFF),
					(0xFF),
					(0xFF),
					(0xFF));
				break; 
					
				case 1: 
				snprintf(UpLoadBuf1,
						 sizeof(UpLoadBuf1)-1,
						 " 8 3 \"%02X:%02X:%02X:%02X\"",
						 (unsigned int)((Flash_Data.Remote_Control_Id[0] >> 24) & 0xFF),
						 (unsigned int)((Flash_Data.Remote_Control_Id[0] >> 16) & 0xFF),
						 (unsigned int)((Flash_Data.Remote_Control_Id[0] >> 8) & 0xFF),
						 (unsigned int)(Flash_Data.Remote_Control_Id[0] & 0xFF));
				break; 

				case 2:
				snprintf(UpLoadBuf1,
						 sizeof(UpLoadBuf1)-1,
						 " 8 3 \"%02X:%02X:%02X:%02X,%02X:%02X:%02X:%02X\"",
						 (unsigned int)((Flash_Data.Remote_Control_Id[0] >> 24) & 0xFF),
						 (unsigned int)((Flash_Data.Remote_Control_Id[0] >> 16) & 0xFF),
						 (unsigned int)((Flash_Data.Remote_Control_Id[0] >> 8) & 0xFF),
						 (unsigned int)(Flash_Data.Remote_Control_Id[0] & 0xFF),
						 (unsigned int)((Flash_Data.Remote_Control_Id[1] >> 24) & 0xFF),
						 (unsigned int)((Flash_Data.Remote_Control_Id[1] >> 16) & 0xFF),
						 (unsigned int)((Flash_Data.Remote_Control_Id[1] >> 8) & 0xFF),
						 (unsigned int)(Flash_Data.Remote_Control_Id[1] & 0xFF));
				break;

				case 3: 
				snprintf(UpLoadBuf1,
						 sizeof(UpLoadBuf1)-1,
						 " 8 3 \"%02X:%02X:%02X:%02X,%02X:%02X:%02X:%02X,%02X:%02X:%02X:%02X\"",
						 (unsigned int)((Flash_Data.Remote_Control_Id[0] >> 24) & 0xFF),
						 (unsigned int)((Flash_Data.Remote_Control_Id[0] >> 16) & 0xFF),
						 (unsigned int)((Flash_Data.Remote_Control_Id[0] >> 8) & 0xFF),
						 (unsigned int)(Flash_Data.Remote_Control_Id[0] & 0xFF),
						 (unsigned int)((Flash_Data.Remote_Control_Id[1] >> 24) & 0xFF),
						 (unsigned int)((Flash_Data.Remote_Control_Id[1] >> 16) & 0xFF),
						 (unsigned int)((Flash_Data.Remote_Control_Id[1] >> 8) & 0xFF),
						 (unsigned int)(Flash_Data.Remote_Control_Id[1] & 0xFF),
						 (unsigned int)((Flash_Data.Remote_Control_Id[2] >> 24) & 0xFF),
						 (unsigned int)((Flash_Data.Remote_Control_Id[2] >> 16) & 0xFF),
						 (unsigned int)((Flash_Data.Remote_Control_Id[2] >> 8) & 0xFF),
						 (unsigned int)(Flash_Data.Remote_Control_Id[2] & 0xFF));
				break;

				default:
				snprintf(UpLoadBuf1,
					sizeof(UpLoadBuf1)-1,
					" 8 3 \"%02X:%02X:%02X:%02X\"",
					(0xFF),
					(0xFF),
					(0xFF),
					(0xFF));
				break;
				}
					Up_State_Data.Control_num=Device_State_Data.Control_num;
					memcpy((uint8_t *)uart1_tx_buf + BufNum, UpLoadBuf1, strlen(UpLoadBuf1));
					BufNum += strlen(UpLoadBuf1);
					ret = TRUE;							
					Property_num++;
					if(Property_num==PACK_SPEC_NUM)
					{
						 goto send_data;
					}	
	}
 
send_data:
	if(ret==TRUE)
	{
		uart1_tx_buf[BufNum] = 0x0d;

		r_uart0_send_bytes((uint8_t *)uart1_tx_buf, BufNum + 1); 
		memset(uart1_tx_buf, 0, sizeof(uart1_tx_buf));
		BufNum=0;
	}
	return ret;
}


void ModuleResetReq(void)//重置模块
{  	
	Led.Led_Blue_state=LED_BLUE_STATE_OFF;
	Time.WIFI_enable_config_time=0;
	Led.Led_Orange_On_time=0;
	Flag.Reset_model=1;
	//出产默认参数
	Flash_Data.Max_Light_Value=100;
	Flash_Data.Light_CT_value=5700;
	colour_temperature_temp=(Flash_Data.Light_CT_value-3000)/5;
	if((Flash_Data.Electronic_Down_Limit<100)&&(Device_State_Data.Motor_State_Byte==MOTOR_DOWN_STOP_STATE))
	{
		Device_State_Data.Motor_State_Byte=MOTOR_SUSPEND_STATE;	
	}
	Flash_Data.Electronic_Down_Limit=100;
	
	if((Flash_Data.Electronic_Up_Limit>0)&&(Device_State_Data.Motor_State_Byte==MOTOR_UP_STOP_STATE))
	{
		Device_State_Data.Motor_State_Byte=MOTOR_SUSPEND_STATE;	
	}	
	Flash_Data.Electronic_Up_Limit=0;
	
	Flash_Data.Light_open_hour_time=23;
	Flash_Data.Light_open_min_time=0;
	Flash_Data.Light_close_hour_time=6;
	Flash_Data.Light_close_min_time=0;	
	
	Flash_Data.Light_Bright_Enable=0;
	Flag.Brightness_time_flag=0;
	Flash_Data.Light_Bright_Value=20;
	Flash_Data.Beep_open=1;
	
	set_down_limit_value=Flash_Data.Electronic_Down_Limit*ONE_PERCENT_PULSE+UP_POSITION_VALUE;//电子下限位对应霍尔脉冲数
	if(Flash_Data.Electronic_Up_Limit==0)
	{
		set_up_limit_value=Flash_Data.Electronic_Up_Limit*ONE_PERCENT_PULSE;//电子上限位对应霍尔脉冲数	
	}
	else
	{
		set_up_limit_value=Flash_Data.Electronic_Up_Limit*ONE_PERCENT_PULSE+1000;
	}	

	save_data();//恢复默认值
	Device_State_Data.Light_Bright=Flash_Data.Max_Light_Value;
	Device_State_Data.Light_colour_temperature=Flash_Data.Light_CT_value;
	
	Device_State_Data.electronic_down_limit=Flash_Data.Electronic_Down_Limit;
	Device_State_Data.electronic_up_limit=Flash_Data.Electronic_Up_Limit;
	
	Device_State_Data.start_time_hour=Flash_Data.Light_open_hour_time;
	Device_State_Data.start_time_min=Flash_Data.Light_open_min_time;
	Device_State_Data.end_time_hour=Flash_Data.Light_close_hour_time;
	Device_State_Data.end_time_min=Flash_Data.Light_close_min_time;
	
	Device_State_Data.nightlight_switch=Flash_Data.Light_Bright_Enable;		
	Device_State_Data.brightness=Flash_Data.Light_Bright_Value;
	Device_State_Data.Beep_Enable=Flash_Data.Beep_open;	
	properties_up();//上报恢复值
	
	memcpy((uint8_t *)uart1_tx_buf, MiioConst.SentReast, strlen(MiioConst.SentReast));
	r_uart0_send_bytes((uint8_t *)uart1_tx_buf, strlen(MiioConst.SentReast));                
	Led.Led_Orange_state=LED_ORG_STATE_TWINK;
}


void Return_Properties(void)//回应app查询属性
{
	uint8_t _data[4];
	uint8_t Num=7;
	char *UpLoadBuf;
	uint8_t value;
	memcpy((uint8_t *)uart1_tx_buf, MiioConst.setresult, 7);//上报命令字	
	for(idx=0;idx<Frame_length-21;idx=idx+offset_value)
  {
			if(uart1_rx_buf[20+idx] == 0x32)
		{
        offset_value=4;
		if((uart1_rx_buf[22+idx]==0x31)&&(uart1_rx_buf[23+idx]==0x32))////ppid>10运行到指定位置
			{

			UpLoadBuf ="2 12 0 15 ";
			memcpy((uint8_t *)uart1_tx_buf + Num, UpLoadBuf, 9);
			Num += 10;
		//	uart0_tx_buf[Num - 3] =_data[0]+0x30;
			//uart0_tx_buf[Num - 2] =_data[1]+0x30;
			uart1_tx_buf[Num - 1]=0x20;
				
			}else 		if((uart1_rx_buf[22+idx]==0x32)&&(uart1_rx_buf[23+idx]==0x30))////ppid>10运行到指定位置
			{

				UpLoadBuf ="2 20 0 0 ";
				memcpy((uint8_t *)uart1_tx_buf + Num, UpLoadBuf, 9);
				Num += 9;
				uart1_tx_buf[Num - 1]=0x20;
			
			}
      else 	if (uart1_rx_buf[22+idx] == 0x31)//查询A、B杆故障
			{
				UpLoadBuf ="2 1 0 0 ";
				memcpy((uint8_t *)uart1_tx_buf + Num, UpLoadBuf, 8);
				Num += 8;
				uart1_tx_buf[Num - 1]=0x20;
			}
			else if (uart1_rx_buf[22+idx] == 0x33)//查询双杆收合开关
			{
				if(Device_State_Data.convergent_all==CONVERGENT_OFF)
				{
					UpLoadBuf="2 3 0 false ";//
					memcpy((uint8_t *)uart1_tx_buf + Num, UpLoadBuf, 12);
					Num += 12;		
				}
				else if(Device_State_Data.convergent_all==CONVERGENT_ON)
				{
					UpLoadBuf="2 3 0 true ";
					memcpy((uint8_t *)uart1_tx_buf + Num, UpLoadBuf, 11);
					Num += 11;			
				}
			}
			else
			{
				break;
			}
		}
		else if(uart1_rx_buf[20+idx] == 0x35)
		{
			offset_value=4;
			if(uart1_rx_buf[22+idx] == 0x31)//查询提示音灯状态
			{
				if (Device_State_Data.Beep_Enable==BEEP_ON)
				{
					UpLoadBuf = "5 1 0 true ";
					memcpy((uint8_t *)uart1_tx_buf + Num, UpLoadBuf, 11);
					Num += 11;		
				}
				else 
				{
					UpLoadBuf = "5 1 0 false ";
					memcpy((uint8_t *)uart1_tx_buf + Num, UpLoadBuf, 12);
					Num += 12;								
				}	
			}
			else
			{
				break;
			}
		}
		else if (uart1_rx_buf[20+idx] == 0x33)//LIGHT
		{	
			offset_value=4;
			if(uart1_rx_buf[22+idx] == 0x31)//查询照明灯状态
			{
				if (Device_State_Data.Light_State_Byte==LIGHT_ON)
				{
					UpLoadBuf = "3 1 0 true ";
					memcpy((uint8_t *)uart1_tx_buf + Num, UpLoadBuf, 11);
					Num += 11;		
				}
				else 
				{
					UpLoadBuf = "3 1 0 false ";
					memcpy((uint8_t *)uart1_tx_buf + Num, UpLoadBuf, 12);
					Num += 12;								
				}	
			}
			else if(uart1_rx_buf[22+idx] == 0x32)//查询当前亮度
			{
				UpLoadBuf = "3 2 0 0 ";
				memcpy((uint8_t *)uart1_tx_buf + Num, UpLoadBuf, 8);
				if(Device_State_Data.Light_Bright<10)
				{
					Num += 8;
					uart1_tx_buf[Num - 2] =Device_State_Data.Light_Bright+0x30;
					uart1_tx_buf[Num - 1]=0x20;
				}
				else if(Device_State_Data.Light_Bright<100)
				{	
					_data[0]=Device_State_Data.Light_Bright/10;
					_data[1]=Device_State_Data.Light_Bright%10;	
					Num += 9;
					uart1_tx_buf[Num - 3] =_data[0]+0x30;
					uart1_tx_buf[Num - 2] =_data[1]+0x30;
					uart1_tx_buf[Num - 1]=0x20;
				}
				else
				{
					Num += 10;
					uart1_tx_buf[Num - 4] =0x31;
					uart1_tx_buf[Num - 3] =0x30;
					uart1_tx_buf[Num - 2] =0x30;
					uart1_tx_buf[Num - 1]=0x20;
				}
			}					
			else if(uart1_rx_buf[22+idx] == 0x35)//查询照明夜灯开关状态
			{
				if (Device_State_Data.nightlight_switch==LIGHT_ON)
				{
					UpLoadBuf = "3 5 0 true ";
					memcpy((uint8_t *)uart1_tx_buf + Num, UpLoadBuf, 11);
					Num += 11;		
				}
				else 
				{
					UpLoadBuf = "3 5 0 false ";
					memcpy((uint8_t *)uart1_tx_buf + Num, UpLoadBuf, 12);
					Num += 12;								
				}	
			}	
			else if(uart1_rx_buf[22+idx] == 0x36)//查询当前亮度
			{
				UpLoadBuf = "3 6 0 0 ";
				memcpy((uint8_t *)uart1_tx_buf + Num, UpLoadBuf, 8);
				if(Device_State_Data.brightness<10)
				{
					Num += 8;
					uart1_tx_buf[Num - 2] =Device_State_Data.brightness+0x30;
					uart1_tx_buf[Num - 1]=0x20;
				}
				else if(Device_State_Data.brightness<100)
				{	
					_data[0]=Device_State_Data.brightness/10;
					_data[1]=Device_State_Data.brightness%10;	
					Num += 9;
					uart1_tx_buf[Num - 3] =_data[0]+0x30;
					uart1_tx_buf[Num - 2] =_data[1]+0x30;
					uart1_tx_buf[Num - 1]=0x20;
				}
				else
				{
					Num += 10;
					uart1_tx_buf[Num - 4] =0x31;
					uart1_tx_buf[Num - 3] =0x30;
					uart1_tx_buf[Num - 2] =0x30;
					uart1_tx_buf[Num - 1]=0x20;
				}
			}
    	else if(uart1_rx_buf[22+idx] == 0x33)//查询当前色温
			{
				UpLoadBuf = "3 3 0 0 ";
				memcpy((uint8_t *)uart1_tx_buf + Num, UpLoadBuf, 8);
				if(Device_State_Data.Light_colour_temperature<10)
				{

					Num += 8;
					uart1_tx_buf[Num - 2] =Device_State_Data.Light_colour_temperature+0x30;
					uart1_tx_buf[Num - 1] =0x20;
				}
				else if(Device_State_Data.Light_colour_temperature<100)
				{	
					_data[0]=Device_State_Data.Light_colour_temperature/10;
					_data[1]=Device_State_Data.Light_colour_temperature%10;	

					Num += 9;
					uart1_tx_buf[Num - 3] =_data[0]+0x30;
					uart1_tx_buf[Num - 2] =_data[1]+0x30;
					uart1_tx_buf[Num - 1] =0x20;
				}
				else if(Device_State_Data.Light_colour_temperature<1000)
				{
					_data[0]=Device_State_Data.Light_colour_temperature/100;
					_data[1]=Device_State_Data.Light_colour_temperature%100/10;
					_data[2]=Device_State_Data.Light_colour_temperature%10;

					Num += 10;
					uart1_tx_buf[Num - 4] =_data[0]+0x30;
					uart1_tx_buf[Num - 3] =_data[1]+0x30;
					uart1_tx_buf[Num - 2] =_data[2]+0x30;
					uart1_tx_buf[Num - 1] =0x20;
				}	
				else if(Device_State_Data.Light_colour_temperature<10000)
				{
					_data[0]=Device_State_Data.Light_colour_temperature/1000;
					_data[1]=Device_State_Data.Light_colour_temperature%1000/100;	
					_data[2]=Device_State_Data.Light_colour_temperature%100/10;	
					_data[3]=Device_State_Data.Light_colour_temperature%10;
					Num += 11;
					uart1_tx_buf[Num - 5] =_data[0]+0x30;
					uart1_tx_buf[Num - 4] =_data[1]+0x30;
					uart1_tx_buf[Num - 3] =_data[2]+0x30;
					uart1_tx_buf[Num - 2] =_data[3]+0x30;
					uart1_tx_buf[Num - 1] =0x20;
				}
			}
	 else if(uart1_rx_buf[22+idx] == 0x34)//查询当前模式
			{

							
				memcpy((uint8_t *)uart1_tx_buf + Num, UpLoadBuf, 8);
				Num += 8;
			
			}
		 else if(uart1_rx_buf[22+idx] == 0x37)//查询定时时间
			{
				UpLoadBuf = "3 7 0 \"00:00:00-00:00:00\" ";
				memcpy((uint8_t *)uart1_tx_buf + Num, UpLoadBuf, 26);
				Num += 26;
				uart1_tx_buf[Num-19]=Device_State_Data.start_time_hour/10+0x30;
				uart1_tx_buf[Num-18]=Device_State_Data.start_time_hour%10+0x30;
				
				uart1_tx_buf[Num-16]=Device_State_Data.start_time_min/10+0x30;
				uart1_tx_buf[Num-15]=Device_State_Data.start_time_min%10+0x30;
				
				uart1_tx_buf[Num-10]=Device_State_Data.end_time_hour/10+0x30;
				uart1_tx_buf[Num-9]=Device_State_Data.end_time_hour%10+0x30;
				
				uart1_tx_buf[Num-7]=Device_State_Data.end_time_min/10+0x30;
				uart1_tx_buf[Num-6]=Device_State_Data.end_time_min%10+0x30;				
				uart1_tx_buf[Num - 1] =0x20;
			}			
			else
			{
				break;
			}	
				
		}
		else if (uart1_rx_buf[20+idx] == 0x37)
		{
			if((uart1_rx_buf[23+idx]<0x30)||(uart1_rx_buf[23+idx]>0x39))//ppid<10
			{
				offset_value=4;
				if (uart1_rx_buf[22+idx] == 0x31)//查询A杆故障
				{
					UpLoadBuf ="7 1 0 0 ";
					memcpy((uint8_t *)uart1_tx_buf + Num, UpLoadBuf, 8);
					Num += 8;
					if(Err.data&0x01)//有遇阻
					{
						uart1_tx_buf[Num-2]=0x31;
					}
					else if(Err.data&0x02)//有超重
					{
						uart1_tx_buf[Num-2]=0x32;
					}
					else if(Err.data&0x04)//有电机堵转
					{
						uart1_tx_buf[Num-2]=0x33;
					}
					else
					{
						uart1_tx_buf[Num-2]=0x30;
					}
					uart1_tx_buf[Num - 1]=0x20;
				}
				else if (uart1_rx_buf[22+idx] == 0x34)//A晾杆状态
				{
					UpLoadBuf = (char *)Up_Motor_Properties[Device_State_Data.Motor_State_Byte];
					memcpy((uint8_t *)uart1_tx_buf + Num, UpLoadBuf, 8);
					Num += 8;
					uart1_tx_buf[Num-2]=Device_State_Data.Motor_State_Byte+0x30;
					uart1_tx_buf[Num - 1]=0x20;
				}
				else 
				{
					break;
				}
			}
			else
			{
				offset_value=5;
				if ((uart1_rx_buf[22+idx]==0x31)&&(uart1_rx_buf[23+idx]==0x31))//A晾杆当前位置
				{
					UpLoadBuf ="7 11 0 0 ";
					memcpy((uint8_t *)uart1_tx_buf + Num, UpLoadBuf, 9);
					value=Device_State_Data.Motor_Position;
					if(value<10)
					{
						Num += 9;
						uart1_tx_buf[Num - 2] =value+0x30;
						uart1_tx_buf[Num - 1]=0x20;
					}
					else if(value<100)
					{	
						_data[0]=value/10;
						_data[1]=value%10;
						
						Num += 10;
						uart1_tx_buf[Num - 3] =_data[0]+0x30;
						uart1_tx_buf[Num - 2] =_data[1]+0x30;
						uart1_tx_buf[Num - 1]=0x20;
					}
					else
					{
						Num += 11;
						uart1_tx_buf[Num - 4] =0x31;
						uart1_tx_buf[Num - 3] =0x30;
						uart1_tx_buf[Num - 2] =0x30;
						uart1_tx_buf[Num - 1]=0x20;
					}
				}				
				else if ((uart1_rx_buf[22+idx]==0x31)&&(uart1_rx_buf[23+idx]==0x32))//查询A晾杆设定位置
				{
					UpLoadBuf ="7 12 0 0 ";
					memcpy((uint8_t *)uart1_tx_buf + Num, UpLoadBuf, 9);	
					value=Device_State_Data.set_position;
					if(value<10)
					{
						Num += 9;
						uart1_tx_buf[Num - 2] =value+0x30;
						uart1_tx_buf[Num - 1]=0x20;
					}
					else if(value<100)
					{	
						_data[0]=value/10;
						_data[1]=value%10;
						
						Num += 10;
						uart1_tx_buf[Num - 3] =_data[0]+0x30;
						uart1_tx_buf[Num - 2] =_data[1]+0x30;
						uart1_tx_buf[Num - 1]=0x20;
					}
					else
					{
						Num += 11;
						uart1_tx_buf[Num - 4] =0x31;
						uart1_tx_buf[Num - 3] =0x30;
						uart1_tx_buf[Num - 2] =0x30;
						uart1_tx_buf[Num - 1]=0x20;
					}
				}	
				else if((uart1_rx_buf[22+idx]==0x31)&&(uart1_rx_buf[23+idx]==0x33))//A杆上限位
				{
					UpLoadBuf ="7 13 0 0 ";
					memcpy((uint8_t *)uart1_tx_buf + Num, UpLoadBuf, 9);	
					value=Device_State_Data.electronic_up_limit;
					if(value<10)
					{
						Num += 9;
						uart1_tx_buf[Num - 2] =value+0x30;
						uart1_tx_buf[Num - 1]=0x20;
					}
					else if(value<100)
					{
						_data[0]=value/10;
						_data[1]=value%10;	
						Num += 10;
						uart1_tx_buf[Num - 3] =_data[0]+0x30;
						
						uart1_tx_buf[Num - 2] =_data[1]+0x30;
						uart1_tx_buf[Num - 1]=0x20;
					}
					else
					{
						Num += 11;
						uart1_tx_buf[Num - 4] =0x31;
						uart1_tx_buf[Num - 3] =0x30;
						uart1_tx_buf[Num - 2] =0x30;
						uart1_tx_buf[Num - 1]=0x20;				
					}	
				}
				else if((uart1_rx_buf[22+idx]==0x31)&&(uart1_rx_buf[23+idx]==0x34))//A杆下限位
				{
					UpLoadBuf ="7 14 0 0 ";
					memcpy((uint8_t *)uart1_tx_buf + Num, UpLoadBuf, 9);	
					value=Device_State_Data.electronic_down_limit;
					if(value<10)
					{
						Num += 9;
						uart1_tx_buf[Num - 2] =value+0x30;
						uart1_tx_buf[Num - 1]=0x20;
					}
					else if(value<100)
					{
						_data[0]=value/10;
						_data[1]=value%10;	
						Num += 10;
						uart1_tx_buf[Num - 3] =_data[0]+0x30;
						uart1_tx_buf[Num - 2] =_data[1]+0x30;
						uart1_tx_buf[Num - 1]=0x20;
					}
					else
					{
						Num += 11;
						uart1_tx_buf[Num - 4] =0x31;
						uart1_tx_buf[Num - 3] =0x30;
						uart1_tx_buf[Num - 2] =0x30;
						uart1_tx_buf[Num - 1]=0x20;				
					}	
				}
				else if((uart1_rx_buf[22+idx]==0x31)&&(uart1_rx_buf[23+idx]==0x35))//A杆收合状态
				{
					if (Device_State_Data.convergent==CONVERGENT_ON)
					{
						UpLoadBuf = "7 15 0 true ";
						memcpy((uint8_t *)uart1_tx_buf + Num, UpLoadBuf, 12);
						Num += 12;		
					}
					else 
					{
						UpLoadBuf ="7 15 0 false ";
						memcpy((uint8_t *)uart1_tx_buf + Num, UpLoadBuf, 13);
						Num += 13;								
					}
				}
				else if((uart1_rx_buf[22+idx]==0x31)&&(uart1_rx_buf[23+idx]==0x36))
				{
					UpLoadBuf = "7 16 0 0 ";
					memcpy((uint8_t *)uart1_tx_buf + Num, UpLoadBuf, 9);
					value=Device_State_Data.current_speed;
					if(value<10)
					{
						Num += 11;
						uart1_tx_buf[Num - 4] =0x30;
						uart1_tx_buf[Num - 3] ='.';						
						uart1_tx_buf[Num - 2] =value+0x30;
						uart1_tx_buf[Num - 1]=0x20;
					}
					else if(value<100)
					{	
						_data[0]=value/10;
						_data[1]=value%10;
						
						Num += 11;
						uart1_tx_buf[Num - 4] =_data[0]+0x30;
						uart1_tx_buf[Num - 3] ='.';
						uart1_tx_buf[Num - 2] =_data[1]+0x30;
						uart1_tx_buf[Num - 1]=0x20;
					}
					else
					{
						_data[0]=value/100;
						_data[1]=value%100/10;
						_data[2]=value%10;
						Num += 12;
						uart1_tx_buf[Num - 5] =_data[0]+0x31;
						uart1_tx_buf[Num - 4] =_data[1]+0x30;
						uart1_tx_buf[Num - 3] ='.';
						uart1_tx_buf[Num - 2] =_data[2]+0x30;
						uart1_tx_buf[Num - 1]=0x20;
					}				
				}

			}
		}
/*遥控器设置*/
		else if (uart1_rx_buf[20+idx] == 0x38) 
		{
			    offset_value=4;//识别的指令+4
				  if((uart1_rx_buf[22+idx]==0x31) ) 
					{
							if (Device_State_Data.Control_open==BEEP_ON)
							{
								UpLoadBuf = "8 1 0 true ";
								memcpy((uint8_t *)uart1_tx_buf + Num, UpLoadBuf, 11);
								Num += 11;		
							}
							else 
							{
								UpLoadBuf = "8 1 0 false ";
								memcpy((uint8_t *)uart1_tx_buf + Num, UpLoadBuf, 12);
								Num += 12;								
							}	
					}
				  else if((uart1_rx_buf[22+idx]==0x37)) // PIID 7: paired remote count
				  {
				  	UpLoadBuf = "8 7 0 ";
				  	memcpy((uint8_t *)uart1_tx_buf + Num, UpLoadBuf, 6);
				  	Num += 6;
				  	uart1_tx_buf[Num - 2] = Flash_Data.Remote_Control_Num + 0x30;
				  	uart1_tx_buf[Num - 1] = 0x20;
				  }
	  else	 if((uart1_rx_buf[22+idx]==0x33)) 
					{
				  char UpLoadBuf1[90];
					memset(UpLoadBuf1, 0, sizeof(UpLoadBuf1));
					switch(Flash_Data.Remote_Control_Num)	
					{
						case 0: 
							
								snprintf(UpLoadBuf1,
								sizeof(UpLoadBuf1)-1,
								"8 3 0 \"%02X:%02X:%02X:%02X \" ",
								(0xFF),
								(0xFF),
								(0xFF),
								(0xFF));
					  
						break; 
					case 1: 
							snprintf(UpLoadBuf1,
									 sizeof(UpLoadBuf1)-1,
									 "8 3 0 \"%02X:%02X:%02X:%02X \" ",
									 (unsigned int)((Flash_Data.Remote_Control_Id[0] >> 24) & 0xFF),
									 (unsigned int)((Flash_Data.Remote_Control_Id[0] >> 16) & 0xFF),
									 (unsigned int)((Flash_Data.Remote_Control_Id[0] >> 8) & 0xFF),
									 (unsigned int)(Flash_Data.Remote_Control_Id[0] & 0xFF));
							break; 

					case 2:
							snprintf(UpLoadBuf1,
									 sizeof(UpLoadBuf1)-1,
									 "8 3 0 \"%02X:%02X:%02X:%02X,%02X:%02X:%02X:%02X\" ",
									 (unsigned int)((Flash_Data.Remote_Control_Id[0] >> 24) & 0xFF),
									 (unsigned int)((Flash_Data.Remote_Control_Id[0] >> 16) & 0xFF),
									 (unsigned int)((Flash_Data.Remote_Control_Id[0] >> 8) & 0xFF),
									 (unsigned int)(Flash_Data.Remote_Control_Id[0] & 0xFF),
									 (unsigned int)((Flash_Data.Remote_Control_Id[1] >> 24) & 0xFF),
									 (unsigned int)((Flash_Data.Remote_Control_Id[1] >> 16) & 0xFF),
									 (unsigned int)((Flash_Data.Remote_Control_Id[1] >> 8) & 0xFF),
									 (unsigned int)(Flash_Data.Remote_Control_Id[1] & 0xFF));
							break;

					case 3: 
							snprintf(UpLoadBuf1,
									 sizeof(UpLoadBuf1)-1,
									 "8 3 0 \"%02X:%02X:%02X:%02X,%02X:%02X:%02X:%02X,%02X:%02X:%02X:%02X\" ",
									 (unsigned int)((Flash_Data.Remote_Control_Id[0] >> 24) & 0xFF),
									 (unsigned int)((Flash_Data.Remote_Control_Id[0] >> 16) & 0xFF),
									 (unsigned int)((Flash_Data.Remote_Control_Id[0] >> 8) & 0xFF),
									 (unsigned int)(Flash_Data.Remote_Control_Id[0] & 0xFF),
									 (unsigned int)((Flash_Data.Remote_Control_Id[1] >> 24) & 0xFF),
									 (unsigned int)((Flash_Data.Remote_Control_Id[1] >> 16) & 0xFF),
									 (unsigned int)((Flash_Data.Remote_Control_Id[1] >> 8) & 0xFF),
									 (unsigned int)(Flash_Data.Remote_Control_Id[1] & 0xFF),
									 (unsigned int)((Flash_Data.Remote_Control_Id[2] >> 24) & 0xFF),
									 (unsigned int)((Flash_Data.Remote_Control_Id[2] >> 16) & 0xFF),
									 (unsigned int)((Flash_Data.Remote_Control_Id[2] >> 8) & 0xFF),
									 (unsigned int)(Flash_Data.Remote_Control_Id[2] & 0xFF));
							break;
 
						default:
						snprintf(UpLoadBuf1,
								sizeof(UpLoadBuf1)-1,
								"8 3 0 \"%02X:%02X:%02X:%02X \" ",
								(0xFF),
								(0xFF),
								(0xFF),
								(0xFF));
						break;
					}

						memcpy((uint8_t *)uart1_tx_buf + Num, UpLoadBuf1, strlen(UpLoadBuf1));
						Num += strlen(UpLoadBuf1);	
					}
 	
		   }
		if(Num>179)//防止数据太长
		{
      Num=179;
			uart1_tx_buf[Num-1] = 0x0d;//结束符	
			r_uart0_send_bytes((uint8_t *)uart1_tx_buf, Num);
			Num=0;
			memset(uart1_tx_buf, 0, sizeof(uart1_tx_buf));
			return ;
		}
 
	}
	uart1_tx_buf[Num-1] = 0x0d;//结束符	
	r_uart0_send_bytes((uint8_t *)uart1_tx_buf, Num);
	Num=0;
	g_MiioRoundFlag = 1;
	Flag.time150ms_flag=0;
	Time.time150ms=150;
	
}
void Set_Properties (void)//回应app控制指令
{
	uint8_t i=0,offset=0;
	uint8_t ii=0;
	uint8_t Num=7;
	char *UpLoadBuf;	
	memcpy((uint8_t *)uart1_tx_buf, MiioConst.setresult, 7);
while((20+i)<Frame_length)
	{
		if(uart1_rx_buf[20+i] == 0x35)
		{
			if(uart1_rx_buf[22+i]==0x31)
			{
				UpLoadBuf = "5 1 0 ";
				memcpy((uint8_t *)uart1_tx_buf+Num, UpLoadBuf, 6);
				Num+=6;
				if ((uart1_rx_buf[24+i] == 0x31) || (uart1_rx_buf[24+i] == 0x74))//打开
				{
					if(uart1_rx_buf[24+i] == 0x31)
					{
						offset=6;
					}
					else
					{
						offset=9;
					}
					Flash_Data.Beep_open = BEEP_ON;
					Device_State_Data.Beep_Enable=Flash_Data.Beep_open;
				}
				else if ((uart1_rx_buf[24+i] == 0x30) || (uart1_rx_buf[24+i] == 0x66))//关闭
				{
					if(uart1_rx_buf[24+i] == 0x30)
					{
						offset=6;
					}
					else
					{
						offset=10;
					}
					Flash_Data.Beep_open = BEEP_OFF;
					Device_State_Data.Beep_Enable=Flash_Data.Beep_open;		
				}
			}
			else
			{
				break;
			}
		}
		else if(uart1_rx_buf[20+i] == 0x34)//指示灯控制
		{
		        break;
		}
		else if (uart1_rx_buf[20+i] == 0x33)//
		{	
			if(uart1_rx_buf[22+i] == 0x31)//照明控制
			{
				UpLoadBuf = "3 1 0 ";
				memcpy((uint8_t *)uart1_tx_buf+Num, UpLoadBuf, 6);
				Num+=6;
				if ((uart1_rx_buf[24+i] == 0x31) || (uart1_rx_buf[24+i] == 0x74))//打开
				{
					if(uart1_rx_buf[24+i] == 0x31)
					{
						offset=6;
					}
					else
					{
						offset=9;
					}
					if (Device_State_Data.Light_State_Byte == LIGHT_OFF)
					{
						Time.Light_key_space_time=Light_KEY_SPACE_TIME;
						Cmd_Light();
					}
				}
				else if ((uart1_rx_buf[24+i] == 0x30) || (uart1_rx_buf[24+i] == 0x66))//关闭
				{
					if(uart1_rx_buf[24+i] == 0x30)
					{
						offset=6;
					}
					else
					{
						offset=10;
					}
					if (Device_State_Data.Light_State_Byte == LIGHT_ON) 
					{
						Time.Light_key_space_time=Light_KEY_SPACE_TIME;
						Cmd_Light();
					}
				}			
			}
			else if(uart1_rx_buf[22+i] == 0x32)//照明亮度
			{
				UpLoadBuf = "3 2 0 ";
				memcpy((uint8_t *)uart1_tx_buf+Num, UpLoadBuf, 6);
				Num+=6;	
				if((uart1_rx_buf[25+i]==0x0d)||(uart1_rx_buf[25+i]==0x20))
				{
					Device_State_Data.Light_Bright=(uart1_rx_buf[24+i]-0x30);	
					offset=6;
				}
				else if((uart1_rx_buf[26+i]==0x0d)||(uart1_rx_buf[26+i]==0x20))
				{
					Device_State_Data.Light_Bright=(uart1_rx_buf[24+i]-0x30)*10+(uart1_rx_buf[25+i]-0x30);
					offset=7;
				}
				else if((uart1_rx_buf[27+i]==0x0d)||(uart1_rx_buf[27+i]==0x20))
				{
					Device_State_Data.Light_Bright=(uart1_rx_buf[24+i]-0x30)*100+(uart1_rx_buf[25+i]-0x30)*10
								    +uart1_rx_buf[26+i]-0x30;
					offset=8;
				}
				if(Device_State_Data.Light_State_Byte==LIGHT_OFF)
				{
					Device_State_Data.Light_State_Byte=LIGHT_ON;
					colour_temperature_temp=(Flash_Data.Light_CT_value-3000)/5;//开灯时色温不渐变，直接是最终状态
					SetupBzhx(LIGHT_ON_BEEP,Flash_Data.Beep_open);
				}
				else
				{
					//SetupBzhx(LONG_BEEP,Flash_Data.Beep_open);
				}	
				//在夜灯开启后,照明滑条不改变日灯记忆亮度
				if((Flash_Data.Light_Bright_Enable==1)&&(Flag.Brightness_time_flag))
				{
					Flag.Brightness_change=1;//夜灯模式下照明滑条动作
				}
				else
				{
					Flash_Data.Max_Light_Value=Device_State_Data.Light_Bright;
				}
			}
			else if(uart1_rx_buf[22+i] == 0x34)//灯光色温
			{
				UpLoadBuf = "3 4 0 ";
				memcpy((uint8_t *)uart1_tx_buf+Num, UpLoadBuf, 6);
				Num+=6;		
				offset=6;
				if((uart1_rx_buf[25+i]==0x0d)||(uart1_rx_buf[25+i]==0x20))
				{
					Device_State_Data.Light_colour_temperature=(uart1_rx_buf[24+i]-0x30);	
					offset=6;
				}
				else if((uart1_rx_buf[26+i]==0x0d)||(uart1_rx_buf[26+i]==0x20))
				{
					Device_State_Data.Light_colour_temperature=(uart1_rx_buf[24+i]-0x30)*10+(uart1_rx_buf[25+i]-0x30);
					offset=7;
				}
				else if((uart1_rx_buf[27+i]==0x0d)||(uart1_rx_buf[27+i]==0x20))
				{
					Device_State_Data.Light_colour_temperature=(uart1_rx_buf[24+i]-0x30)*100+(uart1_rx_buf[25+i]-0x30)*10
								    +uart1_rx_buf[26+i]-0x30;
					offset=8;
				}
				else if((uart1_rx_buf[28+i]==0x0d)||(uart1_rx_buf[28+i]==0x20))
				{
					Device_State_Data.Light_colour_temperature=(uart1_rx_buf[24+i]-0x30)*1000+(uart1_rx_buf[25+i]-0x30)*100
								    +(uart1_rx_buf[26+i]-0x30)*10+uart1_rx_buf[27+i]-0x30;
					offset=9;					
				}
				if(Device_State_Data.Light_State_Byte==LIGHT_OFF)
				{
					Device_State_Data.Light_State_Byte=LIGHT_ON;
					colour_temperature_temp=(Device_State_Data.Light_colour_temperature-3000)/5;//开灯时色温不渐变，直接是最终状态
					SetupBzhx(LIGHT_ON_BEEP,Flash_Data.Beep_open);
				}
				else
				{
					//SetupBzhx(LONG_BEEP,Flash_Data.Beep_open);
				}
				Flash_Data.Light_CT_value=Device_State_Data.Light_colour_temperature;
			}
			else if(uart1_rx_buf[22+i] == 0x35)//夜灯开关
			{
				UpLoadBuf = "3 5 0 ";
				memcpy((uint8_t *)uart1_tx_buf+Num, UpLoadBuf, 6);
				Num+=6;		
				offset=6;
				if ((uart1_rx_buf[24+i] == 0x31) || (uart1_rx_buf[24+i] == 0x74))//打开
				{
					if(uart1_rx_buf[24+i] == 0x31)
					{
						offset=6;
					}
					else
					{
						offset=9;
					}
					Device_State_Data.nightlight_switch=LIGHT_ON;
					Flash_Data.Light_Bright_Enable=Device_State_Data.nightlight_switch;
				}
				else if ((uart1_rx_buf[24+i] == 0x30) || (uart1_rx_buf[24+i] == 0x66))//关闭
				{
					if(uart1_rx_buf[24+i] == 0x30)
					{
						offset=6;
					}
					else
					{
						offset=10;
					}
					Device_State_Data.nightlight_switch=LIGHT_OFF;
					Flash_Data.Light_Bright_Enable=Device_State_Data.nightlight_switch;
				}	
				Judge_Nightlight_Mode();
				if(Flag.Brightness_time_flag)//进入夜灯时间
				{
					Device_State_Data.Light_Bright=Flash_Data.Light_Bright_Value;
				}
				else
				{
					Device_State_Data.Light_Bright=Flash_Data.Max_Light_Value;
				}
			}
			else if(uart1_rx_buf[22+i] == 0x36)//夜灯亮度
			{
				UpLoadBuf = "3 6 0 ";
				memcpy((uint8_t *)uart1_tx_buf+Num, UpLoadBuf, 6);
				Num+=6;		
				if((uart1_rx_buf[25+i]==0x0d)||(uart1_rx_buf[25+i]==0x20))
				{
					Device_State_Data.brightness=(uart1_rx_buf[24+i]-0x30);	
					offset=6;
				}
				else if((uart1_rx_buf[26+i]==0x0d)||(uart1_rx_buf[26+i]==0x20))
				{
					Device_State_Data.brightness=(uart1_rx_buf[24+i]-0x30)*10+(uart1_rx_buf[25+i]-0x30);
					offset=7;
				}
				else if((uart1_rx_buf[27+i]==0x0d)||(uart1_rx_buf[27+i]==0x20))
				{
					Device_State_Data.brightness=(uart1_rx_buf[24+i]-0x30)*100+(uart1_rx_buf[25+i]-0x30)*10
								    +uart1_rx_buf[26+i]-0x30;
					offset=8;
				}
				Flash_Data.Light_Bright_Value=Device_State_Data.brightness;
				Judge_Nightlight_Mode();
				if(Flag.Brightness_time_flag)//进入夜灯时间
				{
					Device_State_Data.Light_Bright=Flash_Data.Light_Bright_Value;
				}
				else
				{
					Device_State_Data.Light_Bright=Flash_Data.Max_Light_Value;
				}				
			}
			else if(uart1_rx_buf[22+i] == 0x39)//夜灯起始时间,第24个字节开始"00:00:00-00:00:00"
			{
				UpLoadBuf = "3 9 0 ";
				memcpy((uint8_t *)uart1_tx_buf+Num, UpLoadBuf, 6);
				Num+=6;	
				offset=6;
				Device_State_Data.start_time_hour=(uart1_rx_buf[25+i]-0x30)*10+(uart1_rx_buf[26+i]-0x30);
				Device_State_Data.start_time_min=(uart1_rx_buf[28+i]-0x30)*10+(uart1_rx_buf[29+i]-0x30);
				
				Device_State_Data.end_time_hour=(uart1_rx_buf[34+i]-0x30)*10+(uart1_rx_buf[35+i]-0x30);
				Device_State_Data.end_time_min=(uart1_rx_buf[37+i]-0x30)*10+(uart1_rx_buf[38+i]-0x30);
				
				Flash_Data.Light_open_hour_time=Device_State_Data.start_time_hour;
				Flash_Data.Light_open_min_time=Device_State_Data.start_time_min;
				Flash_Data.Light_close_hour_time=Device_State_Data.end_time_hour;
				Flash_Data.Light_close_min_time=Device_State_Data.end_time_min;
				
				Judge_Nightlight_Mode();
				
				if(Flag.Brightness_time_flag)//进入夜灯时间
				{
					Device_State_Data.Light_Bright=Flash_Data.Light_Bright_Value;
				}
				else
				{
					Device_State_Data.Light_Bright=Flash_Data.Max_Light_Value;
				}	
			}
			else
			{
				break;
			}
		}
		else if (uart1_rx_buf[20+i] == 0x32)//
		{
			if((uart1_rx_buf[23+i]<0x30)||(uart1_rx_buf[23+i]>0x39))//ppid<10
			{
				if(uart1_rx_buf[22+i] == 0x35)//电机控制
				{
					UpLoadBuf = "2 5 0 ";
					memcpy((uint8_t *)uart1_tx_buf+Num, UpLoadBuf, 6);
					Num+=6;
					offset=6;
					switch (uart1_rx_buf[24+i])
					{
						case 0x30:
						{
							Cmd_Motor_Stop();
						}break;		 
						case 0x31:
						{
							if(Up_State_Data.Err_Byte&0x01)
							{
								
								Up_State_Data.Motor_State_Byte=MOTOR_UP_STATE;//app会点亮图标，如果此时是遇阻，需要上报停止
								Err.err_bit.Encounter_Obstacle=0;
								Time.Encounter_Obstacle_beepnum=0;
								Up_State_Data.Err_Byte&=0xfe;//重新上报app遇阻
							}
							else
							{
								Flag.No_judge_electronic_limit=0;
								if(Device_State_Data.Motor_State_Byte!=MOTOR_UP_STATE)
								{
									Cmd_Motor_Up();
								}
							}
						}break;		
						case 0x32:
						{
							if(Up_State_Data.Err_Byte&0x01)
							{
								Up_State_Data.Motor_State_Byte=MOTOR_DOWN_STATE;//app会点亮图标，如果此时是遇阻，需要上报停止
								Err.err_bit.Encounter_Obstacle=0;
								Time.Encounter_Obstacle_beepnum=0;
								Up_State_Data.Err_Byte&=0xfe;//重新上报app遇阻
							}
							else
							{
								Flag.No_judge_electronic_limit=0;
								if(Device_State_Data.Motor_State_Byte!=MOTOR_DOWN_STATE)
								{
									Cmd_Motor_Down();
								}
							}
						}break;
							
						case 0x33:
						{
							Flag.No_judge_electronic_limit=1;
							Cmd_Motor_Stop();
						}break;
						
						case 0x34:
						{
							Flag.No_judge_electronic_limit=1;
							Cmd_Motor_Up();	
						}break;
						case 0x35:
						{	
							if(Ell_Data.Motor_Current_Position<DOWN_POSITION_VALUE)
							{
								Flag.No_judge_electronic_limit=1;
								Cmd_Motor_Down();
							}	
						}break;					
						default:  break;	 
							 											 
					}
				}
				else
				{
					break;
				}
			}
			else
			{
				if((uart1_rx_buf[22+i]==0x31)&&(uart1_rx_buf[23+i]==0x32))//
				{
					UpLoadBuf = "2 12 0 ";
					memcpy((uint8_t *)uart1_tx_buf+Num, UpLoadBuf, 6);
					Num+=7;
					if((uart1_rx_buf[26+i]==0x0d)||(uart1_rx_buf[25+i]==0x20))
					{
						Device_State_Data.set_position=(uart1_rx_buf[25+i]-0x30);	
						offset=7;					
					}
					else if((uart1_rx_buf[27+i]==0x0d)||(uart1_rx_buf[26+i]==0x20))
					{
						Device_State_Data.set_position=(uart1_rx_buf[25+i]-0x30)*10+(uart1_rx_buf[26+i]-0x30);
						offset=8;
					}
					else if((uart1_rx_buf[28+i]==0x0d)||(uart1_rx_buf[27+i]==0x20))
					{
						Device_State_Data.set_position=(uart1_rx_buf[25+i]-0x30)*100+(uart1_rx_buf[26+i]-0x30)*10
									    +uart1_rx_buf[27+i]-0x30;
						offset=9;
					}
					Run_to_position();	
				}
				else if((uart1_rx_buf[22+i]==0x31)&&(uart1_rx_buf[23+i]==0x33))//设置上限位
				{
					UpLoadBuf = "2 13 0 ";
					memcpy((uint8_t *)uart1_tx_buf+Num, UpLoadBuf, 7);
					Num+=7;		
					if((uart1_rx_buf[26+i]==0x0d)||(uart1_rx_buf[26+i]==0x20))
					{
						Device_State_Data.electronic_up_limit=(uart1_rx_buf[25+i]-0x30);
						offset=7;
					}
					else if((uart1_rx_buf[27+i]==0x0d)||(uart1_rx_buf[27+i]==0x20))
					{
						Device_State_Data.electronic_up_limit=(uart1_rx_buf[25+i]-0x30)*10+(uart1_rx_buf[26+i]-0x30);
						offset=8;
					}
					else if((uart1_rx_buf[28+i]==0x0d)||(uart1_rx_buf[28+i]==0x20))
					{
						Device_State_Data.electronic_up_limit=(uart1_rx_buf[25+i]-0x30)*100+(uart1_rx_buf[26+i]-0x30)*10
								    +uart1_rx_buf[27+i]-0x30;	
						offset=9;
					}
					Flash_Data.Electronic_Up_Limit=Device_State_Data.electronic_up_limit;	
					if(Flash_Data.Electronic_Up_Limit==0)
					{
						set_up_limit_value=0;//电子上限位对应霍尔脉冲数
					}
					else
					{
						set_up_limit_value=Flash_Data.Electronic_Up_Limit*ONE_PERCENT_PULSE+1000;
					}
					if(Ell_Data.Motor_Current_Position<set_up_limit_value)
					{
						Device_State_Data.Motor_State_Byte=MOTOR_UP_STOP_STATE;
					}
				}
				else if((uart1_rx_buf[22+i]==0x31)&&(uart1_rx_buf[23+i]==0x34))//设置下限位
				{
					UpLoadBuf = "2 14 0 ";
					memcpy((uint8_t *)uart1_tx_buf+Num, UpLoadBuf, 7);
					Num+=7;		
					if((uart1_rx_buf[26+i]==0x0d)||(uart1_rx_buf[26+i]==0x20))
					{
						Device_State_Data.electronic_down_limit=(uart1_rx_buf[25+i]-0x30);
						offset=7;
					}
					else if((uart1_rx_buf[27+i]==0x0d)||(uart1_rx_buf[27+i]==0x20))
					{
						Device_State_Data.electronic_down_limit=(uart1_rx_buf[25+i]-0x30)*10+(uart1_rx_buf[26+i]-0x30);
						offset=8;
					}
					else if((uart1_rx_buf[28+i]==0x0d)||(uart1_rx_buf[28+i]==0x20))
					{
						Device_State_Data.electronic_down_limit=(uart1_rx_buf[25+i]-0x30)*100+(uart1_rx_buf[26+i]-0x30)*10
								    +uart1_rx_buf[27+i]-0x30;	
						offset=9;
					}
					Flash_Data.Electronic_Down_Limit=Device_State_Data.electronic_down_limit;
					set_down_limit_value=Flash_Data.Electronic_Down_Limit*ONE_PERCENT_PULSE+1000;
					Device_State_Data.Motor_State_Byte=MOTOR_DOWN_STOP_STATE;
				}
				else if((uart1_rx_buf[22+i]==0x31)&&(uart1_rx_buf[23+i]==0x36))
				{
					UpLoadBuf = "2 16 0 ";
					memcpy((uint8_t *)uart1_tx_buf+Num, UpLoadBuf, 7);
					Num+=7;		
					if (uart1_rx_buf[25+i] == 0x74)//打开
					{
						offset=10;
						if (Device_State_Data.convergent == CONVERGENT_OFF)
						{
							//Cmd_Motor_Up_Longpress();
						}
					}
					else if(uart1_rx_buf[25+i] == 0x66)
					{
						offset=11;
						if (Device_State_Data.convergent == CONVERGENT_ON)
						{
							Cmd_Motor_Stop();
						}						
					}	
				}
				else
				{
					break;
				}
			}
		}		/*遥控器设置*/
		else if (uart1_rx_buf[20+i] == 0x38) //Remote Control Management
		{
		 	if((uart1_rx_buf[23+i]<0x31))//开启遥控器添码使能
			{
					UpLoadBuf = "8 1 0 ";
					memcpy((uint8_t *)uart1_tx_buf+Num, UpLoadBuf, 6);
					Num+=6;
					offset=6;
					if( !memcmp(uart1_rx_buf + 24, "false", 5) )
					{
		         Device_State_Data.Control_open=0;
				
					} else
					{
			      Flag.Add_Match_Code_flag=1;
						Device_State_Data.Control_open=1;
					}
					
		     SetupBzhx(ADD_MATCH_CODE_BEEP,Flash_Data.Beep_open);
			   Flash_Data.Control_open=Device_State_Data.Control_open;
			}
 
		}
 
		ii++;
		i+=offset;
		if(ii>6)
		{
						memcpy((uint8_t *)uart1_rx_buf, MiioConst.error5002, strlen (MiioConst.error5002));
					r_uart0_send_bytes((uint8_t *)uart1_rx_buf, strlen (MiioConst.error5002)); 
			  return ;
		}
	}
	uart1_tx_buf[Num-1] = 0x0d;
	r_uart0_send_bytes((uint8_t *)uart1_tx_buf, Num);	
  Num=0;
	g_MiioRoundFlag = 1;
	Flag.time150ms_flag=0;
	Time.time150ms=150;
	//上报属性
	//properties_up();
	//
}
void Run_to_position(void)
{	
	if(Device_State_Data.set_position!=Device_State_Data.Motor_Position)//运行目标位置不等于当前位置
	{
		if(Device_State_Data.set_position==0)
		{
			set_position_pulse=0;
			if(Device_State_Data.set_position<Device_State_Data.electronic_up_limit)
			{
				set_position_pulse=Device_State_Data.electronic_up_limit*ONE_PERCENT_PULSE+UP_POSITION_VALUE;	
				if(Device_State_Data.Motor_Position==Device_State_Data.electronic_up_limit)
				{
					set_position_pulse=Ell_Data.Motor_Current_Position;
				}
				Device_State_Data.set_position=Device_State_Data.electronic_up_limit;//设置位置变更为上限位
				Up_State_Data.set_position=0XFF;//重新上报
				Up_State_Data.Motor_Position=0xff;
			}		
		}
		else
		{
			set_position_pulse=Device_State_Data.set_position*ONE_PERCENT_PULSE+UP_POSITION_VALUE;
			if(Device_State_Data.set_position>Device_State_Data.electronic_down_limit)
			{
				set_position_pulse=Device_State_Data.electronic_down_limit*ONE_PERCENT_PULSE+UP_POSITION_VALUE;	
				if(Device_State_Data.Motor_Position==Device_State_Data.electronic_down_limit)
				{
					set_position_pulse=Ell_Data.Motor_Current_Position;
				}
				Device_State_Data.set_position=Device_State_Data.electronic_down_limit;//设置位置变更为下限位
				Up_State_Data.set_position=0XFF;//
				Up_State_Data.Motor_Position=0xff;
			}
			else if(Device_State_Data.set_position<Device_State_Data.electronic_up_limit)
			{
				set_position_pulse=Device_State_Data.electronic_up_limit*ONE_PERCENT_PULSE+UP_POSITION_VALUE;	
				if(Device_State_Data.Motor_Position==Device_State_Data.electronic_up_limit)
				{
					set_position_pulse=Ell_Data.Motor_Current_Position;
				}	
				Device_State_Data.set_position=Device_State_Data.electronic_up_limit;
				Up_State_Data.set_position=0XFF;
				Up_State_Data.Motor_Position=0xff;
			}		
		}
		if(set_position_pulse<Ell_Data.Motor_Current_Position)//需要上升
		{
			if(Err.err_bit.Encounter_Obstacle==1)//遇阻状态重新报遇阻提示音
			{
				Err.err_bit.Encounter_Obstacle=0;
				Time.Encounter_Obstacle_beepnum=0;
				Up_State_Data.Err_Byte&=0xfe;//重新上报遇阻
				
				Device_State_Data.set_position=Device_State_Data.Motor_Position;
				Up_State_Data.set_position=0XFF;
				Up_State_Data.Motor_Position=0xff;
			}
			else
			{
				Flag.Run_to_set_position_flag=1;
				if((Device_State_Data.Motor_State_Byte==MOTOR_UP_STOP_STATE)
			      	||(Device_State_Data.Motor_State_Byte==MOTOR_SUSPEND_STATE)
				||(Device_State_Data.Motor_State_Byte==MOTOR_DOWN_STOP_STATE))//当前是暂停状态
				{
					SetupBzhx(MOTOR_UP_BEEP,Flash_Data.Beep_open);
					Device_State_Data.Motor_State_Byte=MOTOR_UP_STATE;
					
					
					
					
					Flag.Motor_run_now=0;
				}
				else if(Device_State_Data.Motor_State_Byte==MOTOR_DOWN_STATE)//当前是下降状态，换向
				{
					Device_State_Data.Motor_State_Byte=MOTOR_UP_STATE;
					Flag.Motor_run_now=0;
					//SetupBzhx(MOTOR_UP_BEEP);//换向完再响蜂鸣器
					Flag.Motor_down_to_up=1;
				}
				else if(Device_State_Data.Motor_State_Byte==MOTOR_UP_STATE)//当前是上升
				{
					SetupBzhx(MOTOR_UP_BEEP,Flash_Data.Beep_open);
				}
			}
		}
		else if(set_position_pulse>Ell_Data.Motor_Current_Position)//需要下降
		{
			if(Err.err_bit.Encounter_Obstacle==1)//遇阻状态重新报遇阻提示音
			{
				Err.err_bit.Encounter_Obstacle=0;
				Time.Encounter_Obstacle_beepnum=0;
				Up_State_Data.Err_Byte&=0xfe;//重新上报遇阻
				
				Device_State_Data.set_position=Device_State_Data.Motor_Position;//设置位置不成功，
				Up_State_Data.set_position=0XFF;//重新上报
				Up_State_Data.Motor_Position=0xff;//
			}
			else
			{
				Flag.Run_to_set_position_flag=1;
				if((Device_State_Data.Motor_State_Byte==MOTOR_UP_STOP_STATE)
			      	||(Device_State_Data.Motor_State_Byte==MOTOR_SUSPEND_STATE)
				||(Device_State_Data.Motor_State_Byte==MOTOR_DOWN_STOP_STATE))//当前是暂停状态
				{
					Device_State_Data.Motor_State_Byte=MOTOR_DOWN_STATE;
					SetupBzhx(MOTOR_DOWN_BEEP,Flash_Data.Beep_open);
					Flag.Motor_run_now=0;
				}
				else if(Device_State_Data.Motor_State_Byte==MOTOR_UP_STATE)//当前是上升状态，先停止
				{
					Device_State_Data.Motor_State_Byte=MOTOR_DOWN_STATE;
					//SetupBzhx(MOTOR_DOWN_BEEP);//换向完再响蜂鸣器	
					Flag.Motor_up_to_down=1;
					Flag.Motor_run_now=0;
				}
				else if(Device_State_Data.Motor_State_Byte==MOTOR_DOWN_STATE)//当前是下降
				{
					SetupBzhx(MOTOR_DOWN_BEEP,Flash_Data.Beep_open);
				}
			}
		}
	}
	else//暂停
	{
		if((Device_State_Data.Motor_State_Byte==MOTOR_UP_STOP_STATE)
	      	||(Device_State_Data.Motor_State_Byte==MOTOR_SUSPEND_STATE)
		||(Device_State_Data.Motor_State_Byte==MOTOR_DOWN_STOP_STATE))
		{

		}
		else
		{
			SetupBzhx(MOTOR_STOP_BEEP,Flash_Data.Beep_open);
			Device_State_Data.Motor_State_Byte=MOTOR_SUSPEND_STATE;
			Flag.Motor_run_now=0;		
		}
	}
}


void UpMCUversion(void)
{
	 memcpy((uint8_t *)uart1_tx_buf, MiioConst.MCUversion, strlen (MiioConst.MCUversion));
	 r_uart0_send_bytes((uint8_t *)uart1_tx_buf, strlen (MiioConst.MCUversion));//上报版本	
}
/*设置指令*/
void Action_deal(void)
{
	uint8_t i=0,offset=0,ii=0;
	uint8_t Num=7;
	char *UpLoadBuf;
	memcpy((uint8_t *)uart1_tx_buf, MiioConst.setresult, 7);//上报命令字	
	while((12+i)<(Frame_length-3))
	{
		offset=4;
		if((uart1_rx_buf[12+i]==0x33)&&(uart1_rx_buf[14+i]==0x31))
		{
			UpLoadBuf="3 1 0 ";
			memcpy((uint8_t *)uart1_tx_buf + Num, UpLoadBuf, 6);	
			Num+=6;
 		  Cmd_Light();
		}
		else if((uart1_rx_buf[12+i]==0x37)&&(uart1_rx_buf[14+i]==0x31))//设置A杆上限位
		{
			UpLoadBuf="7 1 0 ";
			memcpy((uint8_t *)uart1_tx_buf + Num, UpLoadBuf, 6);
			Num+=6;
			if(Ell_Data.Motor_Current_Position<=UP_POSITION_VALUE)
			{
				Device_State_Data.electronic_up_limit=0;
			}
			else
			{	
				Device_State_Data.electronic_up_limit=Device_State_Data.Motor_Position;
			}
			Flash_Data.Electronic_Up_Limit=Device_State_Data.electronic_up_limit;	
			if(Flash_Data.Electronic_Up_Limit==0)
			{
				set_up_limit_value=0;//电子上限位对应霍尔脉冲数
			}
			else
			{
				set_up_limit_value=Flash_Data.Electronic_Up_Limit*ONE_PERCENT_PULSE+UP_POSITION_VALUE;
			}
			if(Ell_Data.Motor_Current_Position<set_up_limit_value)
			{
				Device_State_Data.Motor_State_Byte=MOTOR_UP_STOP_STATE;
			}	
		}	

		else if((uart1_rx_buf[12]==0x37)&&(uart1_rx_buf[14]==0x32))//设置A杆下限位
		{
			UpLoadBuf="7 2 0 ";
			memcpy((uint8_t *)uart1_tx_buf + Num, UpLoadBuf, 6);
			Num+=6;
			if(Ell_Data.Motor_Current_Position>=DOWN_POSITION_VALUE)
			{
				Device_State_Data.electronic_down_limit=100;
			}
			else
			{
				Device_State_Data.electronic_down_limit=Device_State_Data.Motor_Position;
			}
			Flash_Data.Electronic_Down_Limit=Device_State_Data.electronic_down_limit;
			set_down_limit_value=Flash_Data.Electronic_Down_Limit*ONE_PERCENT_PULSE+UP_POSITION_VALUE;//电子下限位对应霍尔脉冲数		
			Device_State_Data.Motor_State_Byte=MOTOR_DOWN_STOP_STATE;		
		}	else if((uart1_rx_buf[12]==0x38)&&(uart1_rx_buf[14]==0x31))//清码
		{
			UpLoadBuf="8 1 0 ";
			memcpy((uint8_t *)uart1_tx_buf + Num, UpLoadBuf, 6);
			Num+=6;
			//memset(&Capture_Command, 0, sizeof(Capture_Command));  		// 整个结构体全部清 0
			SetupBzhx(LONG_BEEP, Flash_Data.Beep_open);
			match_data_clear();  
		 }
    ii++;
		 if(ii>4)
		 {
			uart1_tx_buf[Num-1] = 0x0d;
			r_uart0_send_bytes((uint8_t *)uart1_tx_buf, Num);	
			return ;
		 }
		i+=offset;
	}
	uart1_tx_buf[Num-1] = 0x0d;

}


void Judge_Nightlight_Mode(void)
{
	min_data_now = _hour * 60 + _min;
	min_start_data_flash = Flash_Data.Light_open_hour_time * 60 + Flash_Data.Light_open_min_time;
	min_stop_data_flash  = Flash_Data.Light_close_hour_time * 60 + Flash_Data.Light_close_min_time;

	if (Flash_Data.Light_Bright_Enable == 1)
	{
		if ((_hour < 24) && (_min < 60))
		{
			if (min_start_data_flash < min_stop_data_flash)
			{
				if ((min_start_data_flash <= min_data_now) && (min_data_now < min_stop_data_flash))
					Flag.Brightness_time_flag = 1;
				else
					{ Flag.Brightness_time_flag = 0; Flag.Brightness_change = 0; }
			}
			else if (min_start_data_flash > min_stop_data_flash)
			{
				if ((min_data_now >= min_stop_data_flash) && (min_data_now < min_start_data_flash))
					{ Flag.Brightness_time_flag = 0; Flag.Brightness_change = 0; }
				else
					Flag.Brightness_time_flag = 1;
			}
			else
			{
				if (min_data_now == min_start_data_flash)
					Flag.Brightness_time_flag = 1;
				else
					{ Flag.Brightness_time_flag = 0; Flag.Brightness_change = 0; }
			}
		}
	}
	else
	{
		Flag.Brightness_time_flag = 0;
		Flag.Brightness_change   = 0;
	}
}


/********************************************************************************
* Function Name: Model_Net_Change
* Description  : Miio Net state change (long format)
********************************************************************************/

void Model_Net_Change(void)
{
	if (strcmp(MiioConst.WIFIoffline, (char *)uart1_rx_buf) == 0)
	{
		if (Net_state != NET_OFFLINE)
		{
			if ((Net_state == NET_UAP) || (Net_state == NET_UNPROV))
			{
				Time.WIFI_enable_config_time = 0;
				Led.Light_Led_state = 3;
				if (Device_State_Data.Light_Bright == 30)
					colour_temperature_temp = 0;
				else
					brightness_temp = 30;
				Light_colour_temperature_gear = 0;
				LIGHT_WHITE_VALUE  = (uint16_t)(brightness_temp) * Light_colour_temperature_gear * 3 / 100;
				LIGHT_YELLOW_VALUE = (uint16_t)(brightness_temp) * (540 - Light_colour_temperature_gear) * 3 / 100;
				LED_LIGHT_ON;
			}
			Net_state = NET_OFFLINE;
		}
	}
	else if (strcmp(MiioConst.WIFIuap, (char *)uart1_rx_buf) == 0)
	{
		if (Net_state != NET_UAP)
		{
			if (Flag.Reset_model == 0)
			{
				if (Net_state == NET_CLOUD)
				{
					SetupBzhx(WIFI_RESET_BEEP, Flash_Data.Beep_open);
					Flash_Data.Light_open_hour_time = 23;
					Flash_Data.Light_open_min_time  = 0;
					Flash_Data.Light_close_hour_time = 6;
					Flash_Data.Light_close_min_time  = 0;
					Flash_Data.Light_Bright_Enable = 0;
					Flag.Brightness_time_flag = 0;
					Flash_Data.Light_Bright_Value = 20;
					Device_State_Data.start_time_hour = Flash_Data.Light_open_hour_time;
					Device_State_Data.start_time_min  = Flash_Data.Light_open_min_time;
					Device_State_Data.end_time_hour   = Flash_Data.Light_close_hour_time;
					Device_State_Data.end_time_min    = Flash_Data.Light_close_min_time;
					Device_State_Data.nightlight_switch = Flash_Data.Light_Bright_Enable;
					Device_State_Data.brightness       = Flash_Data.Light_Bright_Value;
					Judge_Nightlight_Mode();
					if (Flag.Brightness_time_flag)
						Device_State_Data.Light_Bright = Flash_Data.Light_Bright_Value;
					else
						Device_State_Data.Light_Bright = Flash_Data.Max_Light_Value;
				}
			}
			Flag.Reset_model = 0;
			Net_state = NET_UAP;
		}
	}
	else if (strcmp(MiioConst.WIFIunprov, (char *)uart1_rx_buf) == 0)
	{
		if (Net_state != NET_UNPROV)
			Net_state = NET_UNPROV;
	}
}

/********************************************************************************
* Function Name: Model_Net_Now
* Description  : Miio Net state change (short format)
********************************************************************************/
void Model_Net_Now(void)
{
	if (strcmp(MiioConst.offline, (char *)uart1_rx_buf) == 0)
	{
		/* LED control for offline state currently disabled */
	}
	else if (strcmp(MiioConst.uap, (char *)uart1_rx_buf) == 0)
	{
		if (Net_state != NET_UAP)
		{
			Net_state = NET_UAP;
			Flag.Reset_model = 0;
		}
	}
	else if (strcmp(MiioConst.cloud, (char *)uart1_rx_buf) == 0)
	{
		if (Net_state != NET_CLOUD)
		{
			Net_state = NET_CLOUD;
			Time._150ms_Count = 156;
			Flag.Reset_model = 0;
			Led.Light_Led_state = 0;
			if (Device_State_Data.Light_State_Byte == LIGHT_OFF)
				LED_LIGHT_OFF;
		}
	}
	else if (strcmp(MiioConst.unprov, (char *)uart1_rx_buf) == 0)
	{
		if (Net_state != NET_UNPROV)
			Net_state = NET_UNPROV;
	}
}
