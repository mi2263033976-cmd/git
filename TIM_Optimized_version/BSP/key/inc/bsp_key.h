#ifndef __BSP_KEY_H__
#define __BSP_KEY_H__

//******************************* Includes *******************************//
#include "main.h"
#include "tim.h"          /* TIM3 句柄 htim3 */
#include "FreeRTOS.h"    /* ← 必须第一个 */
#include "task.h"        /* taskENTER_CRITICAL */
#include "queue.h"       /* QueueHandle_t */
#include "bsp_led.h"
//******************************* Includes *******************************//

//******************************** Defines **********************************//
typedef enum
{
    KEY_EDGE_FALL = 0,          /* 按下沿(下降沿) */
    KEY_EDGE_RISE = 1,          /* 松开沿(上升沿) */
} key_edge_t;   

typedef enum                   /* v2 风格：失败能说出为什么 */
{
    KEY_OK = 0, KEY_ERROR, KEY_ERRORRESOURCE, KEY_ERRORPARAMETER
} key_status_t;

typedef struct
{
    key_edge_t edge;               /* 按键沿类型 */
    uint32_t    tick;          /* 该边沿发生的系统时刻（1 tick = 1ms） */
} key_event_t;

#define KEY_LONG_PRESS_MS      1000   /* 长按阈值(ms)：按下超过该值判长按 */
#define KEY_GLITCH_MS       20U     /* 小于它的"按下-松开"对视为抖动，丢弃 */

extern QueueHandle_t key_queue;    /* key 模块要往这里发事件 */
//******************************** Defines **********************************//

//******************************** Declaring ********************************//
key_status_t key_init(void);
void key_task_func(void *argument);

//******************************** Declaring ********************************//

#endif /* __BSP_KEY_H__ */
