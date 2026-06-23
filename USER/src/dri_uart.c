#include "main.h"
 // 全局变量（放在文件头部）
unsigned char *uart_tx_ptr = NULL;   // 保存src_p
unsigned char uart_tx_len  = 0;      // 保存dataNum
uint8_t *queue_in;
uint8_t *queue_out;

char  uart0_queue_buf[QUEUE_BUF_DEEP];

uint8_t uart1_tx_buf[256];
uint8_t uart1_rx_buf[256];

void log_init(void)
{
	
  	 
    GPIO_InitType GPIO_InitStructure;
    UART_InitType USART_InitStructure;

    GPIO_InitStruct(&GPIO_InitStructure);

    RCC_EnableAPBPeriphClk(LOG_PERIPH | LOG_PERIPH_GPIO, ENABLE);

    GPIO_InitStructure.Pin            = LOG_TX_PIN;
    GPIO_InitStructure.GPIO_Mode      = GPIO_MODE_AF_PP;
    GPIO_InitStructure.GPIO_Alternate = LOG_ALT;
    GPIO_InitPeripheral(LOG_GPIO, &GPIO_InitStructure);

    GPIO_InitStructure.Pin             = LOG_RX_PIN;//配置上拉
  //	GPIO_InitStructure.GPIO_Mode      = GPIO_MODE_INPUT;
    GPIO_InitStructure.GPIO_Alternate  = LOG_ALT;
    GPIO_InitPeripheral(LOG_GPIO, &GPIO_InitStructure);

    USART_InitStructure.BaudRate            = 115200;
    USART_InitStructure.WordLength          = UART_WL_8B;
    USART_InitStructure.StopBits            = UART_STPB_1;
    USART_InitStructure.Parity              = UART_PE_NO;
    USART_InitStructure.Mode                = UART_MODE_RX | UART_MODE_TX;
    USART_InitStructure.OverSampling        = UART_16OVER;

    // init uart
    UART_Init(LOG_UARTx, &USART_InitStructure);
    /* Enable UARTy Receive and Transmit interrupts */
		UART_ConfigInt(LOG_UARTx, UART_INT_RXDNE, ENABLE);
   // UART_ConfigInt(LOG_UARTx, UART_INT_TXDE, ENABLE);
    // enable uart
    UART_Enable(LOG_UARTx, ENABLE);
}

static int is_lr_sent = 0;
 
int fputc(int ch, FILE* f)
{
    if (ch == '\r')
    {
        is_lr_sent = 1;
    }
    else if (ch == '\n')
    {
        if (!is_lr_sent)
        {
            /* Loop until the end of transmission */
            while (UART_GetFlagStatus(LOG_UARTx, UART_FLAG_TXDE) == RESET)//UART_FLAG_TXC  会进入PVD中断
            {
            }
            UART_SendData(LOG_UARTx, (uint8_t)'\r');
        }
        is_lr_sent = 0;
    }
    else
    {
        is_lr_sent = 0;
    }
    /* Loop until the end of transmission */
    while (UART_GetFlagStatus(LOG_UARTx, UART_FLAG_TXDE) == RESET)  //UART_FLAG_TXC  会进入PVD中断
    {
    }
    UART_SendData(LOG_UARTx, (uint8_t)ch);
    return ch;
} 
   
void r_uart0_protocol_init(void)
{
  queue_in = (unsigned char *)uart0_queue_buf;
  queue_out = (unsigned char *)uart0_queue_buf;
}

 uint8_t mcu_common_uart_data_unpack(uint8_t data)
{
  uint8_t ret = FALSE;
    if(Time.Uart_Receive_Interval_time>=RECEIVE_DATA_PACKET_INTERVAL_TIME)
    {
        UART_RX_Count=0;
    }
    Time.Uart_Receive_Interval_time=0;

    if(UART_RX_Count >= sizeof(uart1_rx_buf))
    {
        UART_RX_Count=0;
        Flag.Uart1_Err = 1;
        ret = TRUE;
    }
 
    // 先存数据，再判断结束符
    uart1_rx_buf[UART_RX_Count]=data;
    if(uart1_rx_buf[UART_RX_Count]==0x0D)
    {
        // 此时UART_RX_Count是当前字节的下标，帧长度 = 当前下标 + 1
        Frame_length = UART_RX_Count + 1;
        UART_RX_Count=0;
        ret = TRUE;
    }
    else
    {
        // 只有不是结束符时才自增
        UART_RX_Count++;
    }
 
    return ret;
}

unsigned char get_queue_total_data(void)
{  
if(queue_in != queue_out)
    return 1;
  else
    return 0;
}


unsigned char Queue_Read_Byte(void)
{
  unsigned char value;
  
  if(queue_out != queue_in)
  {
    //Data is not empty
    if(queue_out >= (unsigned char *)(uart0_queue_buf + sizeof(uart0_queue_buf)))
    {
      //Data has reached the end
      queue_out = (unsigned char *)(uart0_queue_buf);
    }    
    value = *queue_out ++;   
  } 
  return value;
}


void r_uart0_send_bytes(unsigned char *src_p,unsigned char dataNum)
{
	uint8_t i=0;

	for(i=0;i<dataNum;i++)
	{
  
    /* 等待DR寄存器空，才能填入下一字节 */
		while(UART_GetFlagStatus(LOG_UARTx, UART_FLAG_TXDE) == RESET);
		UART_SendData(LOG_UARTx, *src_p++);
				
		send_busy=1;
	}
}
void r_uart0_receive_input(unsigned char value)
{
  if((queue_in > queue_out) && ((queue_in - queue_out) >= sizeof(uart0_queue_buf)))
  {
	// the data queue is full
  }
  else if((queue_in < queue_out) && ((queue_out  - queue_in) == 0))
  {
    //Data queue is full
  }
  else
  {
    //Queue is not full
    if(queue_in >= (unsigned char *)(uart0_queue_buf + sizeof(uart0_queue_buf)))
    {
      queue_in = (unsigned char *)(uart0_queue_buf);
    }  
    *queue_in ++ = value;
  }
}