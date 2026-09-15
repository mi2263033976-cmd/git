/******************************************************************************
 * Copyright (C) 2026 <BUBUGou>
 *
 * All Rights Reserved.
 *
 * @file bsp_key.c
 *
 * @par dependencies
 * - bsp_key.h
 *
 * @author <BUBUGou> | <班级/部门> | <学校/公司>
 *
 * @brief Provide the polling key scan APIs.
 *
 * Processing flow:
 *
 * call key_scan() periodically from a task every 10ms.
 *
 * @version V1.0 2026-09-07
 *
 * @note 1 tab == 4 spaces!
 *
 *****************************************************************************/

#include "bsp_key.h"

//******************************** Defines **********************************//

#define KEY_DEBOUNCE_CNT_MAX   2   /* 连续2次一致才确认电平变化(~20ms) */


/* EXTI 触发方向由两个寄存器控制：
   RTSR —— Rising Trigger Selection（哪位=1 就使能哪个引脚的上升沿）
   FTSR —— Falling Trigger Selection（下降沿）
   这里"改成只认某一个沿"，就是"使能一个、清掉另一个"。 */
#define KEY_EXTI_SET_FALLING()  do { EXTI->FTSR |=  KEY_Pin; EXTI->RTSR &= ~KEY_Pin; } while (0)
#define KEY_EXTI_SET_RISING()   do { EXTI->RTSR |=  KEY_Pin; EXTI->FTSR &= ~KEY_Pin; } while (0)

//******************************** Defines **********************************//

//******************************** Static variables *************************//


//******************************** Static variables *************************//

//******************************** Functions ********************************//
osMessageQueueId_t key_queue = NULL;

/* 由 HAL 调用：库里的 HAL_GPIO_EXTI_IRQHandler() 会先清 pending 位，再回调这里 */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    if (KEY_Pin == GPIO_Pin)                 /* ① 共享回调，必须判引脚 */
    {
        key_event_t evt;
        evt.tick = osKernelGetTickCount();   /* ② 取边沿时刻（ISR 安全） */

        if (0U == (EXTI->FTSR & KEY_Pin))    /* ③ 现在"在等上升沿"？→ 那这次是松开 */
        {
            evt.edge = KEY_EDGE_RISE;
            KEY_EXTI_SET_FALLING();          /* 切回下降沿，等下一次按下 */
        }
        else                                 /* 现在"在等下降沿" → 这次是按下 */
        {
            evt.edge = KEY_EDGE_FALL;
            KEY_EXTI_SET_RISING();           /* 切走：抖动期间不再有任何沿可触发 */
        }

        if (NULL != key_queue)               /* ④ 队列没建好就丢弃（防启动竞态） */
        {
            (void)osMessageQueuePut(key_queue, &evt, 0U, 0U);   /* ⑤ 发事件，不等待 */
        }
    }
}

void key_task_func(void *argument)
{
    key_event_t evt;
    key_event_t press_evt = { .edge = KEY_EDGE_FALL, .tick = 0U };
    uint8_t     has_press = 0U;            /* 状态：0=等按下，1=已按下、等松开 */
    (void)argument;

    key_queue = osMessageQueueNew(8U, sizeof(key_event_t), NULL);

    for (;;)
    {
        /* 阻塞等事件：没按键时任务睡着 */
        if (osOK != osMessageQueueGet(key_queue, &evt, NULL, osWaitForever))
        {
            continue;
        }

        /* 状态机：按下 → 只记时刻 */
        if ((KEY_EDGE_FALL == evt.edge) && (0U == has_press))
        {
            press_evt = evt;                       /* t1 = evt.tick */
            has_press = 1U;
        }
        /* 状态机：松开 → 判定（本步只打印，Step 5 再发给 LED） */
        else if ((KEY_EDGE_RISE == evt.edge) && (has_press))
        {
            uint32_t dt = evt.tick - press_evt.tick;   /* t2 − t1（无符号减法） */
            has_press = 0U;

            if (dt < KEY_GLITCH_MS)                    /* 抖动造出的假"按下-松开" */
            {
                continue;
            }

            log_printf("[KEY] dt=%lu -> %s\r\n",
                       (unsigned long)dt,
                       (dt >= KEY_LONG_PRESS_MS) ? "LONG" : "CLICK");
        }
    }
}

//******************************** Functions ********************************//

