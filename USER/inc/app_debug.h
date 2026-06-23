/**
 * @file    app_debug.h
 * @author  Nsing
 * @version v1.0.0
 * @brief   软件可靠性分析调试日志模块
 * @details 提供结构化分级调试日志，支持模块过滤和运行时控制，
 *          输出到晾衣机上位机进行软件可靠性分析。
 *
 *  日志格式: [tick_ms][LEVEL][MOD] message\r\n
 *  示例:     [00123450][INFO][MOTOR] Motor state: UP_STOP -> UP (pos=45%)
 *
 *  使用方式:
 *    DEBUG_TRACE(MOD_MOTOR, "Cmd_Motor_Up called, pos=%d", position);
 *    DEBUG_INFO(MOD_ADC,  "ADC ch0=%d, current=%.2fA", adc_val, current);
 *    DEBUG_WARN(MOD_LIMIT,"Unexpected limit: up=%d down=%d", up, down);
 *    DEBUG_ERROR(MOD_MOTOR,"Motor stall detected! current=%d", adc);
 *    DEBUG_HEXDUMP(MOD_UART, buf, len);
 */

#ifndef __APP_DEBUG_H__
#define __APP_DEBUG_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <string.h>
#include <stdint.h>

/*===========================================================================
 * 主开关: 注释掉以完全移除调试代码 (节省flash/ram)
 *===========================================================================*/
#define DEBUG_ENABLE

/*===========================================================================
 * 调试模块定义 (可按需增删)
 *===========================================================================*/
#define MOD_MAIN      "MAIN"    /**< 主循环/系统 */
#define MOD_MOTOR     "MOTOR"   /**< 电机控制 */
#define MOD_ADC       "ADC"     /**< ADC采样/电流检测 */
#define MOD_LIMIT     "LIMIT"   /**< 限位检测 */
#define MOD_UART      "UART"    /**< 串口通信 */
#define MOD_WIFI      "WIFI"    /**< WiFi/MIIO协议 */
#define MOD_433       "RF433"   /**< 433MHz遥控 */
#define MOD_FLASH     "FLASH"   /**< Flash存储 */
#define MOD_BUZZ      "BUZZ"    /**< 蜂鸣器 */
#define MOD_LED       "LED"     /**< LED照明 */
#define MOD_PVD       "PVD"     /**< 掉电检测 */
#define MOD_TIMER     "TIMER"   /**< 定时器 */
#define MOD_PCBA      "PCBA"    /**< PCBA自检 */
#define MOD_KEY       "KEY"     /**< 按键输入 */

/*===========================================================================
 * 日志等级 (数字越大越详细)
 *===========================================================================*/
#define DBG_LEVEL_NONE    0     /**< 关闭所有输出 */
#define DBG_LEVEL_ERROR   1     /**< 仅错误 */
#define DBG_LEVEL_WARN    2     /**< 错误 + 警告 */
#define DBG_LEVEL_INFO    3     /**< 错误 + 警告 + 关键信息 */
#define DBG_LEVEL_TRACE   4     /**< 全部输出(含函数调用追踪) */

/*===========================================================================
 * 运行时日志等级 (可通过上位机命令动态修改)
 *===========================================================================*/
#ifndef DBG_RUNTIME_LEVEL
#define DBG_RUNTIME_LEVEL  DBG_LEVEL_INFO
#endif

/*===========================================================================
 * 全局tick计数器 (用于时间戳, 在it_timer.c中递增)
 *===========================================================================*/
extern volatile uint32_t g_debug_tick_ms;

/*===========================================================================
 * 运行时控制变量
 *===========================================================================*/
extern uint8_t  g_dbg_level;          /**< 当前日志等级 */
extern uint8_t  g_dbg_module_mask;    /**< 预留: 模块位掩码过滤 */
extern uint8_t  g_dbg_enabled;        /**< 总开关: 1=启用 */

/*===========================================================================
 * API 函数
 *===========================================================================*/
void debug_init(void);
void debug_set_level(uint8_t level);
void debug_set_enabled(uint8_t en);
void debug_output(uint8_t level, const char *mod, const char *fmt, ...);
void debug_hexdump(const char *mod, const uint8_t *data, uint16_t len);
void debug_process_command(const char *cmd);

/*===========================================================================
 * 条件编译宏: DEBUG_ENABLE 未定义时所有宏展开为空
 *===========================================================================*/
#ifdef DEBUG_ENABLE

/* 基础输出宏 */
#define DEBUG_LOG(level, mod, fmt, ...) \
    do { \
        if (g_dbg_enabled && (level <= g_dbg_level)) { \
            debug_output(level, mod, fmt, ##__VA_ARGS__); \
        } \
    } while(0)

/* 便捷宏 - 按等级 */
#define DEBUG_ERROR(mod, fmt, ...)  DEBUG_LOG(DBG_LEVEL_ERROR, mod, fmt, ##__VA_ARGS__)
#define DEBUG_WARN(mod, fmt, ...)   DEBUG_LOG(DBG_LEVEL_WARN,  mod, fmt, ##__VA_ARGS__)
#define DEBUG_INFO(mod, fmt, ...)   DEBUG_LOG(DBG_LEVEL_INFO,  mod, fmt, ##__VA_ARGS__)
#define DEBUG_TRACE(mod, fmt, ...)  DEBUG_LOG(DBG_LEVEL_TRACE, mod, fmt, ##__VA_ARGS__)

/* Hex dump */
#define DEBUG_HEXDUMP(mod, data, len) \
    do { \
        if (g_dbg_enabled && (DBG_LEVEL_TRACE <= g_dbg_level)) { \
            debug_hexdump(mod, data, len); \
        } \
    } while(0)

/* 函数进入/退出追踪 (仅在TRACE级别) */
#define DEBUG_FUNC_ENTER(mod)   DEBUG_TRACE(mod, "-> %s enter", __func__)
#define DEBUG_FUNC_EXIT(mod)    DEBUG_TRACE(mod, "<- %s exit", __func__)

/* 状态变化日志 */
#define DEBUG_STATE_CHANGE(mod, name, old_val, new_val) \
    DEBUG_INFO(mod, "%s: %d -> %d", name, (int)(old_val), (int)(new_val))

/* 断言式日志: 条件为假时打印错误 */
#define DEBUG_ASSERT(mod, cond, fmt, ...) \
    do { \
        if (!(cond)) { \
            DEBUG_ERROR(mod, "ASSERT FAIL: " fmt, ##__VA_ARGS__); \
        } \
    } while(0)

#else  /* !DEBUG_ENABLE - 所有宏展开为空 */

#define DEBUG_LOG(level, mod, fmt, ...)       ((void)0)
#define DEBUG_ERROR(mod, fmt, ...)            ((void)0)
#define DEBUG_WARN(mod, fmt, ...)             ((void)0)
#define DEBUG_INFO(mod, fmt, ...)             ((void)0)
#define DEBUG_TRACE(mod, fmt, ...)            ((void)0)
#define DEBUG_HEXDUMP(mod, data, len)         ((void)0)
#define DEBUG_FUNC_ENTER(mod)                 ((void)0)
#define DEBUG_FUNC_EXIT(mod)                  ((void)0)
#define DEBUG_STATE_CHANGE(mod, name, o, n)   ((void)0)
#define DEBUG_ASSERT(mod, cond, fmt, ...)     ((void)0)

#endif /* DEBUG_ENABLE */

#ifdef __cplusplus
}
#endif

#endif /* __APP_DEBUG_H__ */
