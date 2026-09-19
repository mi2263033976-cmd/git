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
