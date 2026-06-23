/**
 * @file    dri_uart2.h
 * @author  Nsing
 * @version v1.0.0
 * @brief   UART2 驱动层头文件 — 硬件初始化、环形队列、发送接口
 */

#ifndef __DRI_UART2_H__
#define __DRI_UART2_H__

#include "main.h"

#ifdef __cplusplus
extern "C" {
#endif

/*==============================================================================
 * UART2 硬件引脚定义（PA1=TX, PA3=RX, AF4）
 *============================================================================*/
#define UART2_GPIO          GPIOA
#define UART2_PERIPH        RCC_APB_PERIPH_UART2
#define UART2_PERIPH_GPIO   RCC_APB_PERIPH_GPIO
#define UART2_TX_PIN        GPIO_PIN_1
#define UART2_RX_PIN        GPIO_PIN_3
#define UART2_ALT           GPIO_AF4
#define UART2_BAUDRATE      115200U

/*==============================================================================
 * 环形队列配置（大小须为 2 的幂，便于位掩码取模）
 *============================================================================*/
#define UART2_RX_BUF_SIZE   256U
#define UART2_RX_BUF_MASK   (UART2_RX_BUF_SIZE - 1U)

/*==============================================================================
 * 环形队列结构体
 *   head — 写索引，仅 ISR 写入
 *   tail — 读索引，仅主循环写入
 *============================================================================*/
typedef struct {
    uint8_t  buffer[UART2_RX_BUF_SIZE];
    volatile uint16_t head;
    volatile uint16_t tail;
} uart2_ringbuf_t;

/*==============================================================================
 * 全局变量外部声明
 *============================================================================*/
extern uart2_ringbuf_t  uart2_rx_ringbuf;
extern uint8_t          uart2_tx_busy;
extern volatile uint8_t *uart2_tx_ptr;
extern volatile uint16_t uart2_tx_count;

/*==============================================================================
 * 函数声明
 *============================================================================*/
void     uart2_init(uint32_t baudrate);
void     uart2_ringbuf_init(void);
uint8_t  uart2_ringbuf_put(uint8_t data);
uint8_t  uart2_ringbuf_get(uint8_t *data);
uint16_t uart2_ringbuf_available(void);
void     uart2_send_bytes(uint8_t *data, uint16_t len);
void     uart2_send_string(const char *str);

#ifdef __cplusplus
}
#endif

#endif /* __DRI_UART2_H__ */
