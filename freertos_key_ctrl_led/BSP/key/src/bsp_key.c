/********************************************************************************
 * Copyright (C) 2024 EternalChip, Inc.(Gmbh) or its affiliates.
 *
 * ALL Rights Reserved.
 *
 * @file bsp_key.c
 *
 * @par dependencies
 * - bsp_key.h	
 * - stdio.h
 * - stdint.h
 *
 * @author BUBUGou
 *
 * @brief Provide the HAL key of AHT21 and corresponding opetions.
 *
 * Processing flow:
 *
 * call directly.
 *
 * @version V1.0 2026.9.6
 *
 * @note 1 tab == 4 spaces!
 *
 ********************************************************************************/

#include "bsp_key.h"

void bsp_key_init(void)
{
    /* Initialize the key GPIOs */
    // Example: Configure GPIO pins for keys as input with pull-up resistors
    // HAL_GPIO_Init(KEY1_GPIO_PORT, &KEY1_GPIO_InitStruct);
    // HAL_GPIO_Init(KEY2_GPIO_PORT, &KEY2_GPIO_InitStruct);
}