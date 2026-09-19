#include "bsp_key.h"
#include "main.h"         /* 引脚宏定义 (KEY_Pin / KEY_GPIO_Port) */

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
QueueHandle_t key_queue = NULL;

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
   if(KEY_Pin == GPIO_Pin)
   {
      BaseType_t  woken = pdFALSE;   /* ★ 必须初始化 */
      key_event_t evt;
      evt.tick = xTaskGetTickCountFromISR();                  /* ★ ISR 专用 */
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
            (void)xQueueSendToBackFromISR(key_queue, &evt, &woken); /* ★ FromISR */
            portYIELD_FROM_ISR(woken);                              /* ★ 立刻切过去 */
        }
   }
}

/* 队列内存（静态分配）：控制块 cb + 存储区 mem */
static StaticQueue_t key_queue_cb;
static uint8_t       key_queue_mem[10U * sizeof(key_event_t)];

key_status_t key_init(void) //(建队列）
{
    key_queue = xQueueCreateStatic(10U, sizeof(key_event_t), key_queue_mem, &key_queue_cb);
    return (NULL == key_queue) ? KEY_ERRORRESOURCE : KEY_OK;
}

void key_task_func(void *argument)
{
   (void) argument;
   key_event_t evt;
   uint16_t has_press = 0U;
   key_event_t press_evt = { .edge = KEY_EDGE_FALL, .tick = 0U};

   for(;;)
   {
      if(pdTRUE != xQueueReceive(key_queue,&evt,portMAX_DELAY))
      {continue;}
      if((0U == has_press)&&(KEY_EDGE_FALL == evt.edge))
      {
         has_press = 1U;
         press_evt = evt;
      }
      else if((1U == has_press)&&(KEY_EDGE_RISE == evt.edge))
      {
         uint32_t dt = evt.tick - press_evt.tick;
         led_cmd_t c;
         has_press = 0U;

         if(dt < KEY_GLITCH_MS)
         {continue;}
         else if(dt >= KEY_GLITCH_MS )
         {
            c.cmd = (dt >= KEY_LONG_PRESS_MS) ? LED_CMD_LONG : LED_CMD_CLICK;
            c.dt = dt;
         }
         if(0 != led_queue)
         {(void)xQueueSendToBack(led_queue, &c, 0U);}
      }
   }
}

//******************************** Functions ********************************//

