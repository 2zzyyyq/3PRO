/**
 * @file    app_debug.c
 * @author  Nsing
 * @version v1.0.0
 * @brief   软件可靠性分析调试日志模块实现
 * @details 通过 UART2 (PA1 TX) 输出结构化调试日志到晾衣机上位机。
 *          日志格式: [tick_ms][LEVEL][MOD] message\r\n
 *
 *  上位机命令 (通过UART2接收):
 *    DBG:ON       - 启用调试输出
 *    DBG:OFF      - 禁用调试输出
 *    DBG:L0       - 关闭所有日志
 *    DBG:L1       - 仅错误级别
 *    DBG:L2       - 错误 + 警告
 *    DBG:L3       - 错误 + 警告 + 信息
 *    DBG:L4       - 全部输出(含追踪)
 *    DBG:STATUS   - 打印当前调试状态
 */

#include "main.h"
#include "app_debug.h"
#include "dri_uart2.h"
#include <stdarg.h>

/*===========================================================================
 * 全局变量
 *===========================================================================*/
volatile uint32_t g_debug_tick_ms = 0;   /**< 毫秒级tick, 在1ms定时器ISR中递增 */
uint8_t  g_dbg_level        = DBG_RUNTIME_LEVEL;
uint8_t  g_dbg_module_mask  = 0xFF;       /**< 预留: 所有模块使能 */
uint8_t  g_dbg_enabled       = 1;         /**< 默认启用 */

/*===========================================================================
 * 内部缓冲区 (避免栈上大数组)
 *===========================================================================*/
#define DBG_BUF_SIZE  128
static char dbg_buf[DBG_BUF_SIZE];

/*===========================================================================
 * 日志等级字符串映射
 *===========================================================================*/
static const char *level_str(uint8_t level)
{
    switch (level) {
        case DBG_LEVEL_ERROR: return "ERROR";
        case DBG_LEVEL_WARN:  return "WARN ";
        case DBG_LEVEL_INFO:  return "INFO ";
        case DBG_LEVEL_TRACE: return "TRACE";
        default:              return "?????";
    }
}

/*===========================================================================
 * 辅助: 通过 UART2 发送格式化字符串
 *===========================================================================*/
static void dbg_send(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    vsnprintf(dbg_buf, DBG_BUF_SIZE, fmt, args);
    va_end(args);
    uart2_send_string(dbg_buf);
}

/*===========================================================================
 * debug_init - 调试模块初始化
 *===========================================================================*/
void debug_init(void)
{
    g_dbg_enabled = 1;
    g_dbg_level   = DBG_RUNTIME_LEVEL;
    g_debug_tick_ms = 0;

    /* 启动横幅 — 注意: 调用前需确保 uart2_init() 已完成 */
    uart2_send_string("\r\n========================================\r\n");
    dbg_send("[DEBUG] Clothes Dryer Debug Monitor v1.0\r\n");
    dbg_send("[DEBUG] UART2 Output | Level: %s\r\n", level_str(g_dbg_level));
    uart2_send_string("[DEBUG] Commands: DBG:ON/OFF/L0-L4/STATUS\r\n");
    uart2_send_string("========================================\r\n");
}

/*===========================================================================
 * debug_set_level - 运行时修改日志等级
 *===========================================================================*/
void debug_set_level(uint8_t level)
{
    if (level > DBG_LEVEL_TRACE) level = DBG_LEVEL_TRACE;
    g_dbg_level = level;
    dbg_send("[DEBUG] Level changed to: %s (L%d)\r\n",
             level_str(g_dbg_level), g_dbg_level);
}

/*===========================================================================
 * debug_set_enabled - 运行时开关调试
 *===========================================================================*/
void debug_set_enabled(uint8_t en)
{
    g_dbg_enabled = (en != 0) ? 1 : 0;
    if (g_dbg_enabled) {
        dbg_send("[DEBUG] Debug output ENABLED, level=%s\r\n",
                 level_str(g_dbg_level));
    } else {
        uart2_send_string("[DEBUG] Debug output DISABLED\r\n");
    }
}

/*===========================================================================
 * debug_output - 核心格式化输出函数
 * @param level: 日志等级 (DBG_LEVEL_xxx)
 * @param mod:   模块名 (MOD_xxx 字符串)
 * @param fmt:   printf 格式串
 * @param ...:   可变参数
 *===========================================================================*/
void debug_output(uint8_t level, const char *mod, const char *fmt, ...)
{
    va_list args;
    int     head_len;
    int     body_len;

    if (!g_dbg_enabled || level > g_dbg_level) return;
    if (mod == NULL) mod = "????";
    if (fmt == NULL) return;

    /* 1. 构建头部: [tick][LEVEL][MOD] */
    head_len = snprintf(dbg_buf, DBG_BUF_SIZE,
                        "[%08lu][%s][%-5s] ",
                        (unsigned long)g_debug_tick_ms,
                        level_str(level),
                        mod);

    /* 2. 追加消息体 */
    if (head_len < DBG_BUF_SIZE - 4) {
        va_start(args, fmt);
        body_len = vsnprintf(dbg_buf + head_len,
                             DBG_BUF_SIZE - head_len - 3,
                             fmt, args);
        va_end(args);
    } else {
        body_len = 0;
    }

    /* 3. 追加 \r\n 结尾 */
    {
        int total = head_len + body_len;
        if (total >= DBG_BUF_SIZE - 2) {
            total = DBG_BUF_SIZE - 3;
        }
        dbg_buf[total]     = '\r';
        dbg_buf[total + 1] = '\n';
        dbg_buf[total + 2] = '\0';
    }

    /* 4. 通过 UART2 输出 */
    uart2_send_string(dbg_buf);
}

/*===========================================================================
 * debug_hexdump - 十六进制数据转储
 * @param mod:  模块名
 * @param data: 数据指针
 * @param len:  数据长度
 *
 * 输出格式:
 *   [tick][TRACE][MOD  ] HEX[16]: AA BB CC DD EE FF 00 11 22 33 44 55 66 77 88 99
 *===========================================================================*/
void debug_hexdump(const char *mod, const uint8_t *data, uint16_t len)
{
    uint16_t i;
    int      pos;

    if (!g_dbg_enabled || DBG_LEVEL_TRACE > g_dbg_level) return;
    if (data == NULL || len == 0) return;

    for (i = 0; i < len; i += 16) {
        pos = snprintf(dbg_buf, DBG_BUF_SIZE,
                       "[%08lu][TRACE][%-5s] HEX[%03u]: ",
                       (unsigned long)g_debug_tick_ms,
                       mod ? mod : "HEX",
                       (unsigned int)(i / 16));

        {
            uint16_t j;
            for (j = 0; j < 16 && (i + j) < len; j++) {
                if (pos < DBG_BUF_SIZE - 5) {
                    pos += snprintf(dbg_buf + pos,
                                    DBG_BUF_SIZE - pos,
                                    "%02X ", data[i + j]);
                }
            }
        }

        if (pos < DBG_BUF_SIZE - 2) {
            dbg_buf[pos]     = '\r';
            dbg_buf[pos + 1] = '\n';
            dbg_buf[pos + 2] = '\0';
        }

        uart2_send_string(dbg_buf);
    }
}

/*===========================================================================
 * debug_process_command - 解析上位机调试命令
 * @param cmd: 接收到的命令字符串 (以 \r\n 结尾)
 *
 * 支持的命令:
 *   DBG:ON      - 启用调试输出
 *   DBG:OFF     - 禁用调试输出
 *   DBG:L0      - 日志等级: 关闭
 *   DBG:L1      - 日志等级: 仅错误
 *   DBG:L2      - 日志等级: 错误+警告
 *   DBG:L3      - 日志等级: 错误+警告+信息
 *   DBG:L4      - 日志等级: 全部
 *   DBG:STATUS  - 打印当前状态
 *   DBG:HELP    - 打印帮助
 *===========================================================================*/
void debug_process_command(const char *cmd)
{
    if (cmd == NULL) return;

    /* DBG:ON */
    if (strncmp(cmd, "DBG:ON", 6) == 0) {
        debug_set_enabled(1);
    }
    /* DBG:OFF */
    else if (strncmp(cmd, "DBG:OFF", 7) == 0) {
        debug_set_enabled(0);
    }
    /* DBG:L0 ~ DBG:L4 */
    else if (strncmp(cmd, "DBG:L", 5) == 0 && cmd[5] >= '0' && cmd[5] <= '4') {
        debug_set_level(cmd[5] - '0');
    }
    /* DBG:STATUS */
    else if (strncmp(cmd, "DBG:STATUS", 10) == 0) {
        dbg_send("[DEBUG] STATUS: enabled=%d, level=%s (L%d), tick=%lu ms\r\n",
                 g_dbg_enabled,
                 level_str(g_dbg_level),
                 g_dbg_level,
                 (unsigned long)g_debug_tick_ms);
    }
    /* DBG:HELP */
    else if (strncmp(cmd, "DBG:HELP", 8) == 0) {
        uart2_send_string("[DEBUG] Commands:\r\n");
        uart2_send_string("  DBG:ON/OFF  - Enable/disable debug output\r\n");
        uart2_send_string("  DBG:L0      - Silence all\r\n");
        uart2_send_string("  DBG:L1      - Errors only\r\n");
        uart2_send_string("  DBG:L2      - Errors + Warnings\r\n");
        uart2_send_string("  DBG:L3      - Errors + Warnings + Info\r\n");
        uart2_send_string("  DBG:L4      - All (trace)\r\n");
        uart2_send_string("  DBG:STATUS  - Show current status\r\n");
    }
}
