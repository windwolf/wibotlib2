#pragma once

#if defined(STM32G0xx)

#include "stm32g0xx_hal.h"
#include "stm32g0xx_hal_adc.h"
#include "stm32g0xx_hal_dac.h"
#include "stm32g0xx_hal_i2c.h"
#include "stm32g0xx_hal_rcc.h"
#include "stm32g0xx_hal_spi.h"
#include "stm32g0xx_hal_uart.h"
#include "stm32g0xx_ll_adc.h"
#include "stm32g0xx_ll_dac.h"
#include "stm32g0xx_ll_dma.h"
#include "stm32g0xx_ll_gpio.h"
#include "stm32g0xx_ll_i2c.h"
#include "stm32g0xx_ll_rcc.h"
#include "stm32g0xx_ll_spi.h"
#include "stm32g0xx_ll_tim.h"
#include "stm32g0xx_ll_usart.h"

#define RCC_CFGR_PPRE_DIV1 (0x00000000U)

#define AXI_BUFFER
#define RAM1_BUFFER
#define RAM2_BUFFER
#define RAM3_BUFFER
#define RAM4_BUFFER
#define BACKUP_BUFFER

#define AXI_DATA
#define RAM1_DATA
#define RAM2_DATA
#define RAM3_DATA
#define RAM4_DATA
#define BACKUP_DATA

#endif