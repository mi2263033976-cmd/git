/******************************************************************************
 * Copyright (C) 2026 <BUBUGou>
 *
 * All Rights Reserved.
 *
 * @file bsp_log.h
 *
 * @par dependencies
 * - stdio.h / stdarg.h             snprintf、va_list
 * - FreeRTOS.h / task.h / queue.h  QueueHandle_t
 * - usart.h                        huart1
 *
 * @author <BUBUGou> | <班级/部门> | <学校/公司>
 *
 * @brief 日志：分级 + 队列 + 独占串口任务（调用者不阻塞、输出不交叉）。
 *
 * Processing flow:
 *
 * 调用者用 LOG_E/W/I/D 宏 → 先判等级，够则 log_post() 当场把参数格式化成
 * log_item_t 并入队（≈µs，绝不阻塞）→ LogTask 从 log_queue 取出条目，
 * 拼成 "[tick][等级][模块] 正文" 独占串口输出。全工程只有 LogTask 写串口，
 * 因此多任务日志天然不交叉。
 *
 * @version V2.0 2026-09-19
 *
 * @note 1 tab == 4 spaces!
 *
 *****************************************************************************/

#ifndef __BSP_LOG_H__
#define __BSP_LOG_H__
//******************************* Includes *******************************//
#include "main.h"    
#include "stdio.h"      /* snprintf */
#include "stdarg.h"     /* va_list */
#include "FreeRTOS.h"
#include "task.h"        /* taskENTER_CRITICAL */
#include "queue.h"       /* QueueHandle_t */
#include "usart.h"      /* UART_HandleTypeDef huart1 */
//******************************* Includes *******************************//

//******************************** Defines **********************************//
#define LOG_E(mod, ...)  do { if (log_get_level() >= LOG_ERROR) { log_post(LOG_ERROR, mod, __VA_ARGS__); } } while (0)
#define LOG_W(mod, ...)  do { if (log_get_level() >= LOG_WARN)  { log_post(LOG_WARN,  mod, __VA_ARGS__); } } while (0)
#define LOG_I(mod, ...)  do { if (log_get_level() >= LOG_INFO)  { log_post(LOG_INFO,  mod, __VA_ARGS__); } } while (0)
#define LOG_D(mod, ...)  do { if (log_get_level() >= LOG_DEBUG) { log_post(LOG_DEBUG, mod, __VA_ARGS__); } } while (0)


typedef enum
{
    LOG_NONE = 0, LOG_ERROR, LOG_WARN, LOG_INFO, LOG_DEBUG
} log_level_t;

typedef enum                   /* v2 风格：失败能说出为什么 */
{
    LOG_OK = 0, LOG_ERRORRESOURCE, LOG_ERRORPARAMETER
} log_status_t;

typedef struct
{
    log_level_t  level;      /* 等级 */
    const char  *module;     /* 模块名："KEY" / "LED" / "SYS" */
    uint32_t     tick;       /* 发生时刻（入队时取） */
    char         text[64];   /* 正文（已格式化，不含前缀） */
} log_item_t;

extern QueueHandle_t log_queue;  


//******************************** Defines **********************************//

//******************************** Declaring ********************************//
log_status_t log_init(void);
void         log_set_level(log_level_t level);
log_level_t  log_get_level(void);
void         log_task_func(void *argument);
void         log_post(log_level_t level, const char *module, const char *fmt, ...);

//******************************** Declaring ********************************//

#endif /* __BSP_LOG_H__ */
