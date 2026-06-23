#ifndef __H_UART_H__
#define __H_UART_H__
#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>

#define LOG_NONE    0
#define LOG_ERROR   10
#define LOG_WARNING 20
#define LOG_INFO    30
#define LOG_DEBUG   40

/*===========================================================================
 * 旧版日志宏 (保持兼容, 映射到printf)
 * 新代码推荐使用 app_debug.h 中的 DEBUG_xxx 宏, 可获得:
 *   - 时间戳    [tick_ms]
 *   - 等级标签  [ERROR/WARN/INFO/TRACE]
 *   - 模块标签  [MOTOR/ADC/LIMIT/...]
 *   - 运行时动态等级控制
 *===========================================================================*/
#ifndef LOG_LEVEL
#define LOG_LEVEL LOG_DEBUG
#endif

#if LOG_LEVEL >= LOG_INFO
#define log_info(...) printf(__VA_ARGS__)
#else
#define log_info(...)
#endif

#if LOG_LEVEL >= LOG_ERROR
#define log_error(...) printf(__VA_ARGS__)
#else
#define log_error(...)
#endif

#if LOG_LEVEL >= LOG_WARNING
#define log_warning(...) printf(__VA_ARGS__)
#else
#define log_warning(...)
#endif

#if LOG_LEVEL >= LOG_DEBUG
#define log_debug(...) printf(__VA_ARGS__)
#else
#define log_debug(...)
#endif

void log_init(void);
int fputc(int ch, FILE* f);

/*===========================================================================
 * 新版结构化调试日志 (推荐使用)
 *===========================================================================*/
#include "app_debug.h"

#ifdef __cplusplus
}
#endif

#endif
