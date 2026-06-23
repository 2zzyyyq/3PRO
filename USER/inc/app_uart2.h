/**
 * @file    app_uart2.h
 * @author  Nsing
 * @version v1.0.0
 * @brief   UART2 应用层头文件 — 帧解包、数据处理、服务调度
 */

#ifndef __APP_UART2_H__
#define __APP_UART2_H__

#include "main.h"

#ifdef __cplusplus
extern "C" {
#endif

/*==============================================================================
 * 帧缓冲区配置
 *============================================================================*/
#define UART2_FRAME_BUF_SIZE    128U
#define UART2_FRAME_TIMEOUT_MS  20U         /* 帧间隔超时(ms)，与 UART1 一致 */

/*==============================================================================
 * 全局变量外部声明
 *============================================================================*/
extern uint8_t  uart2_frame_buf[UART2_FRAME_BUF_SIZE];
extern uint16_t uart2_frame_len;
extern uint16_t uart2_rx_count;

/*==============================================================================
 * 函数声明
 *============================================================================*/
void    uart2_protocol_init(void);
void    uart2_service(void);
uint8_t uart2_data_unpack(uint8_t data);
void    uart2_data_handle(uint8_t *frame, uint16_t len);

#ifdef __cplusplus
}
#endif

#endif /* __APP_UART2_H__ */
