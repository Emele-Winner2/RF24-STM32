/*
 * RF24_config.h
 *
 *  Created on: 22 Aug 2026
 *      Author: Ekemini Udofia
 */

#ifndef INC_RF24_CONFIG_H_
#define INC_RF24_CONFIG_H_


#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "main.h"

#ifdef __cplusplus
extern "C" {
#endif

// Universal STM32 HAL Inclusions
#if defined(STM32F0)
    #include "stm32f0xx_hal.h"
#elif defined(STM32F1)
    #include "stm32f1xx_hal.h"
#elif defined(STM32F2)
    #include "stm32f2xx_hal.h"
#elif defined(STM32F3)
    #include "stm32f3xx_hal.h"
#elif defined(STM32F4)
    #include "stm32f4xx_hal.h"
#elif defined(STM32F7)
    #include "stm32f7xx_hal.h"
#elif defined(STM32G0)
    #include "stm32g0xx_hal.h"
#elif defined(STM32G4)
    #include "stm32g4xx_hal.h"
#elif defined(STM32H7)
    #include "stm32h7xx_hal.h"
#elif defined(STM32L0)
    #include "stm32l0xx_hal.h"
#elif defined(STM32L1)
    #include "stm32l1xx_hal.h"
#elif defined(STM32L4)
    #include "stm32l4xx_hal.h"
#elif defined(STM32L5)
    #include "stm32l5xx_hal.h"
#elif defined(STM32WB)
    #include "stm32wbxx_hal.h"
#elif defined(STM32WL)
    #include "stm32wlxx_hal.h"
#elif defined(STM32C0)
    #include "stm32c0xx_hal.h"
#elif defined(STM32U5)
    #include "stm32u5xx_hal.h"
#else
    // #error "RF24: Target STM32 family not defined."
#endif

#ifndef _BV
#define _BV(x) (1UL << (x))
#endif

#ifndef min
#define min(a,b) ((a) < (b) ? (a) : (b))
#endif

#ifdef __cplusplus
}
#endif

#endif /* INC_RF24_CONFIG_H_ */
