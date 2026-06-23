/**
 * @file    dri_uart2.c
 * @author  Nsing
 * @version v1.0.0
 * @brief   UART2 驱动层 — 硬件初始化、环形队列（head/tail 索引）、发送接口
 * @note    环形队列采用 head/tail 索引 + 位掩码取模，无锁设计：
 *          head 仅由 ISR 写入，tail 仅由主循环写入。
 */

#include "dri_uart2.h"

/*==============================================================================
 * 环形队列实例
 *============================================================================*/
uart2_ringbuf_t uart2_rx_ringbuf;

/*==============================================================================
 * TX 发送变量（用于中断发送，当前采用轮询发送）
 *============================================================================*/
uint8_t           uart2_tx_busy = 0;
volatile uint8_t *uart2_tx_ptr  = NULL;
volatile uint16_t uart2_tx_count = 0;

/*==============================================================================
 * 环形队列 — 初始化
 *============================================================================*/
void uart2_ringbuf_init(void)
{
    uart2_rx_ringbuf.head = 0;
    uart2_rx_ringbuf.tail = 0;
}

/*==============================================================================
 * 环形队列 — 入队（ISR 上下文调用）
 * 返回 1=成功, 0=队列满
 *============================================================================*/
uint8_t uart2_ringbuf_put(uint8_t data)
{
    uint16_t next_head = (uart2_rx_ringbuf.head + 1) & UART2_RX_BUF_MASK;

    /* 队列满判断：下一个 head 追上 tail 即满 */
    if (next_head == uart2_rx_ringbuf.tail)
    {
        return 0;   /* 队列满，丢弃 */
    }

    uart2_rx_ringbuf.buffer[uart2_rx_ringbuf.head] = data;
    uart2_rx_ringbuf.head = next_head;
    return 1;
}

/*==============================================================================
 * 环形队列 — 出队（主循环上下文调用）
 * 返回 1=成功, 0=队列空
 *============================================================================*/
uint8_t uart2_ringbuf_get(uint8_t *data)
{
    if (uart2_rx_ringbuf.head == uart2_rx_ringbuf.tail)
    {
        return 0;   /* 队列空 */
    }

    *data = uart2_rx_ringbuf.buffer[uart2_rx_ringbuf.tail];
    uart2_rx_ringbuf.tail = (uart2_rx_ringbuf.tail + 1) & UART2_RX_BUF_MASK;
    return 1;
}

/*==============================================================================
 * 环形队列 — 可读字节数
 *============================================================================*/
uint16_t uart2_ringbuf_available(void)
{
    return (uart2_rx_ringbuf.head - uart2_rx_ringbuf.tail) & UART2_RX_BUF_MASK;
}

/*==============================================================================
 * UART2 硬件初始化
 *============================================================================*/
void uart2_init(uint32_t baudrate)
{
    GPIO_InitType GPIO_InitStructure;
    UART_InitType UART_InitStructure;

    GPIO_InitStruct(&GPIO_InitStructure);

    /* 使能 UART2 及 GPIO 时钟 */
    RCC_EnableAPBPeriphClk(UART2_PERIPH | UART2_PERIPH_GPIO, ENABLE);

    /* TX 引脚 — PA1, AF4, 推挽复用 */
    GPIO_InitStructure.Pin            = UART2_TX_PIN;
    GPIO_InitStructure.GPIO_Mode      = GPIO_MODE_AF_PP;
    GPIO_InitStructure.GPIO_Alternate = UART2_ALT;
    GPIO_InitPeripheral(UART2_GPIO, &GPIO_InitStructure);

    /* RX 引脚 — PA3, AF4, 复用 */
    GPIO_InitStructure.Pin            = UART2_RX_PIN;
    GPIO_InitStructure.GPIO_Alternate = UART2_ALT;
    GPIO_InitPeripheral(UART2_GPIO, &GPIO_InitStructure);

    /* UART 参数配置 */
    UART_InitStructure.BaudRate     = baudrate;
    UART_InitStructure.WordLength   = UART_WL_8B;
    UART_InitStructure.StopBits     = UART_STPB_1;
    UART_InitStructure.Parity       = UART_PE_NO;
    UART_InitStructure.Mode         = UART_MODE_RX | UART_MODE_TX;
    UART_InitStructure.OverSampling = UART_16OVER;

    UART_Init(UART2, &UART_InitStructure);

    /* 使能接收中断（RXDNE） */
    UART_ConfigInt(UART2, UART_INT_RXDNE, ENABLE);

    /* 使能 UART2 */
    UART_Enable(UART2, ENABLE);
}

/*==============================================================================
 * UART2 阻塞式发送字节数组
 *============================================================================*/
void uart2_send_bytes(uint8_t *data, uint16_t len)
{
    uint16_t i;
    for (i = 0; i < len; i++)
    {
        /* 等待发送数据寄存器空 */
        while (UART_GetFlagStatus(UART2, UART_FLAG_TXDE) == RESET);
        UART_SendData(UART2, *data++);
    }
}

/*==============================================================================
 * UART2 发送字符串（调试用）
 *============================================================================*/
void uart2_send_string(const char *str)
{
    while (*str != '\0')
    {
        while (UART_GetFlagStatus(UART2, UART_FLAG_TXDE) == RESET);
        UART_SendData(UART2, (uint8_t)*str++);
    }
}
