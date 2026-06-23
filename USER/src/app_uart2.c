/**
 * @file    app_uart2.c
 * @author  Nsing
 * @version v1.0.0
 * @brief   UART2 应用层 — 帧解包、数据分析、主循环服务
 * @note    帧格式：以 \\r (0x0D) 或 \\n (0x0A) 为结束符。
 *          帧间超时 20ms 判定为新帧。
 *          uart2_data_handle() 当前为 echo 回显框架，后续可替换为具体协议。
 */

#include "dri_uart2.h"
#include "app_uart2.h"

/*==============================================================================
 * 帧接收缓冲区
 *============================================================================*/
uint8_t  uart2_frame_buf[UART2_FRAME_BUF_SIZE];
uint16_t uart2_frame_len  = 0;    /* 完整帧长度（含结束符） */
uint16_t uart2_rx_count   = 0;    /* 当前帧已接收字节数 */

/*==============================================================================
 * UART2 应用层初始化
 *============================================================================*/
void uart2_protocol_init(void)
{
    uart2_ringbuf_init();
    uart2_rx_count  = 0;
    uart2_frame_len = 0;
}

/*==============================================================================
 * UART2 主循环服务 — 从环形队列取字节 → 帧解包 → 数据处理
 *============================================================================*/
void uart2_service(void)
{
    uint8_t data;

    while (uart2_ringbuf_get(&data))
    {
        if (uart2_data_unpack(data))
        {
            /* 完整帧就绪，交给数据处理 */
            uart2_data_handle(uart2_frame_buf, uart2_frame_len);
        }
    }
}

/*==============================================================================
 * UART2 帧解包
 * 返回 1 = 完整帧就绪, 0 = 继续接收
 *============================================================================*/
uint8_t uart2_data_unpack(uint8_t data)
{
    uint8_t ret = FALSE;

    /* 帧间超时检测：超过 UART2_FRAME_TIMEOUT_MS 未收到数据则重置 */
    if (Time.Uart2_Receive_Interval_time >= UART2_FRAME_TIMEOUT_MS)
    {
        uart2_rx_count = 0;
    }
    Time.Uart2_Receive_Interval_time = 0;

    /* 缓冲区溢出保护 */
    if (uart2_rx_count >= UART2_FRAME_BUF_SIZE)
    {
        uart2_rx_count = 0;
        ret = TRUE;         /* 溢出也返回，避免丢帧后死等 */
    }

    /* 存入帧缓冲区 */
    uart2_frame_buf[uart2_rx_count] = data;

    /* 检测帧结束符：\\r (0x0D) 或 \\n (0x0A) */
    if (data == 0x0D || data == 0x0A)
    {
        uart2_frame_len = uart2_rx_count + 1;   /* 帧长度 = 下标 + 1 */
        uart2_rx_count  = 0;
        ret = TRUE;
    }
    else
    {
        uart2_rx_count++;
    }

    return ret;
}

/*==============================================================================
 * UART2 数据分析处理（当前为 echo 回显，后续可替换为具体协议）
 *============================================================================*/
void uart2_data_handle(uint8_t *frame, uint16_t len)
{
    /* 回显接收到的帧，方便调试验证 */
    uart2_send_string("[UART2_RX] ");
    uart2_send_bytes(frame, len);
    uart2_send_string("\r\n");
}
