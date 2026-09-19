#include "bsp_log.h"

//******************************** Defines **********************************//

//******************************** Defines **********************************//

//******************************** Static variables *************************//
static const char *level_name(log_level_t lv)
{
    switch (lv)
    {
    case LOG_ERROR: return "ERR";
    case LOG_WARN:  return "WRN";
    case LOG_INFO:  return "INF";
    case LOG_DEBUG: return "DBG";
    default:        return "???";
    }
}

static volatile log_level_t g_log_level = LOG_INFO;    /* 默认 INFO 档 */

//******************************** Static variables *************************//

//******************************** Functions ********************************//
QueueHandle_t log_queue = NULL;

/* 队列内存（静态分配）：控制块 cb + 存储区 mem */
static StaticQueue_t log_queue_cb;
static uint8_t       log_queue_mem[10U * sizeof(log_item_t)];

log_status_t log_init(void)
{
    log_queue = xQueueCreateStatic(10U, sizeof(log_item_t), log_queue_mem, &log_queue_cb);
    return (NULL == log_queue) ?  LOG_ERRORRESOURCE : LOG_OK;
}

void log_task_func(void *argument)
{
    (void)argument;  /* 避免编译器警告 */
    log_item_t item;
    char line[128];
    //test code
    log_post(LOG_INFO, "SYS", "n=%u", 7U);

    for(;;)
    {
        if(pdTRUE != xQueueReceive(log_queue,&item,portMAX_DELAY))
        {continue;}
        int n = snprintf(line, sizeof(line), "[%06lums][%s][%s] %s\r\n",
                        (unsigned long)item.tick, level_name(item.level), item.module, item.text);
        if (n > 0)
        {
            uint16_t len = (n < (int)sizeof(line)) ? (uint16_t)n : (uint16_t)(sizeof(line) - 1);
            (void)HAL_UART_Transmit(&huart1, (uint8_t *)line, len, 100);
        }
    }
}

void log_post(log_level_t level, const char *module, const char *fmt, ...)
{
    log_item_t item;
    va_list ap;

    if(NULL == log_queue)
    {return;}

    item.level = level;
    item.module = module;
    item.tick = xTaskGetTickCount();

    va_start(ap, fmt);
    (void)vsnprintf(item.text, sizeof(item.text), fmt, ap);
    va_end(ap);

    (void)xQueueSendToBack(log_queue, &item, 0U);
}

void log_set_level(log_level_t level)
{
    g_log_level = level;
}

log_level_t log_get_level(void)
{
    return g_log_level;
}

//******************************** Functions ********************************//

