/**
 ******************************************************************************
 * @file    main.h
 * @author  MCU Application Team
 * @brief   Header for main.c file.
 *          This file contains the common defines of the application.
 ******************************************************************************
 * @attention
 *
 * <h2><center>&copy; Copyright (c) 2023 Puya Semiconductor Co.
 * All rights reserved.</center></h2>
 *
 * This software component is licensed by Puya under BSD 3-Clause license,
 * the "License"; You may not use this file except in compliance with the
 * License. You may obtain a copy of the License at:
 *                        opensource.org/licenses/BSD-3-Clause
 *
 ******************************************************************************
 * @attention
 *
 * <h2><center>&copy; Copyright (c) 2016 STMicroelectronics.
 * All rights reserved.</center></h2>
 *
 * This software component is licensed by ST under BSD 3-Clause license,
 * the "License"; You may not use this file except in compliance with the
 * License. You may obtain a copy of the License at:
 *                        opensource.org/licenses/BSD-3-Clause
 *
 ******************************************************************************
 */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "py32f0xx_hal.h"
#include <stdarg.h>

#define KEY_PIN                    (GPIO_PIN_0)
#define KEY_PIN_PORT               (GPIOB)
#define CTRL_PIN                   (GPIO_PIN_3)
#define CTRL_PIN_PORT              (GPIOA)

#define usPerInterrupt             (40)
#define msToTimerInterruptCount(x) ((x) * 1000 / usPerInterrupt)

uint32_t fast_printf(UART_HandleTypeDef *uartHandle, const char *str, ...);
void delay_us(TIM_HandleTypeDef *handle, uint16_t us);
void APP_ErrorHandler(void);

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */

/************************ (C) COPYRIGHT Puya *****END OF FILE****/
