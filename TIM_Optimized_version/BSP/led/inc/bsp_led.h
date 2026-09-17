#ifndef __BSP_LED_H
#define __BSP_LED_H
//******************************* Includes *******************************//

#include "main.h"         /* 引脚宏定义 (LED_Pin / LED_GPIO_Port) */
#include "tim.h"          /* TIM3 句柄 htim3 */
#include "cmsis_os.h"    /* taskENTER_CRITICAL / taskEXIT_CRITICAL */

//******************************* Includes *******************************//

//******************************** Defines **********************************//

#define LED_PWM_PERIOD    2000U   /* = ARR+1 */
#define LED_PWM_DUTY      1000U   /* = CCR，50% */
#define LED_CLICK_CYCLES  1U  //单机翻转1次
#define LED_LONG_CYCLES   10U //长按翻转10次

//******************************** Defines **********************************//

//******************************** Declaring ********************************//

void led_pwm_blink_start(uint16_t cycles);
void led_pwm_tick_handler(void);

//******************************** Declaring ********************************//
#endif

