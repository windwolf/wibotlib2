#pragma once

#include "stm32g431xx.h"
#include "type.hpp"
#include "../port.hpp"

#if defined(STM32F1xx)
#include "stm32f1xx_hal.h"
#include "stm32f1xx_hal_tim.h"
#include "stm32f1xx_ll_adc.h"
#include "stm32f1xx_ll_dac.h"
#include "stm32f1xx_ll_dma.h"
#include "stm32f1xx_ll_gpio.h"
#include "stm32f1xx_ll_i2c.h"
#include "stm32f1xx_ll_rcc.h"
#include "stm32f1xx_ll_spi.h"
#include "stm32f1xx_ll_tim.h"
#include "stm32f1xx_ll_usart.h"

#endif

#if defined(STM32G4xx)
#include "stm32g4xx_hal.h"
#include "stm32g4xx_ll_adc.h"
#include "stm32g4xx_ll_dac.h"
#include "stm32g4xx_ll_dma.h"
#include "stm32g4xx_ll_gpio.h"
#include "stm32g4xx_ll_i2c.h"
#include "stm32g4xx_ll_rcc.h"
#include "stm32g4xx_ll_spi.h"
#include "stm32g4xx_ll_tim.h"
#include "stm32g4xx_ll_usart.h"

constexpr u8 GetTimerAPBIndex(TIM_TypeDef *instance) {
    if (instance == TIM1 || instance == TIM8 || instance == TIM15 || instance == TIM16 ||
        instance == TIM17) {
        return 2;
    } else if (instance == TIM2 || instance == TIM3 || instance == TIM4 || instance == TIM6 ||
               instance == TIM7) {
        return 1;
    } else {
        return 0;
    }
}

#endif

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

#endif

#if defined(STM32H7xx)
#include "stm32h7xx_hal.h"
#include "stm32h7xx_ll_adc.h"
#include "stm32h7xx_ll_dac.h"
#include "stm32h7xx_ll_dma.h"
#include "stm32h7xx_ll_gpio.h"
#include "stm32h7xx_ll_i2c.h"
#include "stm32h7xx_ll_rcc.h"
#include "stm32h7xx_ll_spi.h"
#include "stm32h7xx_ll_tim.h"
#include "stm32h7xx_ll_usart.h"

#endif

/********** ADC **************************/

#ifdef HAL_ADC_MODULE_ENABLED
#define ADC_PER_DECL
#define ADC_CTOR_ARG     ADC_HandleTypeDef &handle
#define ADC_FIELD_DECL   ADC_HandleTypeDef & _handle;
#define ADC_CALLBACK_ARG ADC_HandleTypeDef *handle
#else
#define ADC_PER_DECL
#define ADC_CTOR_ARG     u32 dummy
#define ADC_FIELD_DECL   u32 dummy;
#define ADC_CALLBACK_ARG u32 dummy
#endif  // HAL_ADC_MODULE_ENABLED

/********** ADC **************************/

/********** DAC **************************/

#ifdef HAL_DAC_MODULE_ENABLED
#define DAC_PER_DECL
#define DAC_CTOR_ARG     DAC_HandleTypeDef &handle
#define DAC_FIELD_DECL   DAC_HandleTypeDef & _handle;
#define DAC_CALLBACK_ARG DAC_HandleTypeDef *handle
#else
#define DAC_PER_DECL
#define DAC_CTOR_ARG     u32 dummy
#define DAC_FIELD_DECL   u32 dummy;
#define DAC_CALLBACK_ARG u32 dummy
#endif  // HAL_DAC_MODULE_ENABLED

/********** DAC **************************/

/********** PIN **************************/

#ifdef HAL_GPIO_MODULE_ENABLED
#define PIN_PER_DECL
#define PIN_CTOR_ARG     GPIO_TypeDef *port
#define PIN_FIELD_DECL   GPIO_TypeDef *_port;
#define PIN_CALLBACK_ARG GPIO_TypeDef *port
#else
#define PIN_PER_DECL
#define PIN_CTOR_ARG     u32 dummy
#define PIN_FIELD_DECL   u32 dummy;
#define PIN_CALLBACK_ARG u32 dummy
#endif  // HAL_GPIO_MODULE_ENABLED

/********** PIN **************************/

/********** SD **************************/

#ifdef HAL_SD_MODULE_ENABLED
#define SD_PER_DECL     typedef HAL_SD_CardInfoTypeDef CardInfo;
#define SD_CTOR_ARG     SD_HandleTypeDef &handle
#define SD_FIELD_DECL   SD_HandleTypeDef & _handle;
#define SD_CALLBACK_ARG SD_HandleTypeDef *handle
#else
#define SD_PER_DECL
#define SD_CTOR_ARG     u32 dummy
#define SD_FIELD_DECL   u32 dummy;
#define SD_CALLBACK_ARG u32 dummy
#endif  // HAL_SD_MODULE_ENABLED

/********** SD **************************/

/********** QSPI **************************/

#ifdef HAL_QSPI_MODULE_ENABLED
#define QSPI_PER_DECL
#define QSPI_CTOR_ARG     QSPI_HandleTypeDef &handle
#define QSPI_FIELD_DECL   QSPI_HandleTypeDef & _handle;
#define QSPI_CALLBACK_ARG QSPI_HandleTypeDef *handle
#else
#define QSPI_PER_DECL
#define QSPI_CTOR_ARG     u32 dummy
#define QSPI_FIELD_DECL   u32 dummy;
#define QSPI_CALLBACK_ARG u32 dummy
#endif  // HAL_QSPI_MODULE_ENABLED

/********** QSPI ***************************/
/*-----------------------------------------*/
/********** FLASH **************************/

#ifdef HAL_FLASH_MODULE_ENABLED
constexpr static u32 kBaseAddress = 0x08000000;
constexpr static u32 kFlashSize   = 0x20000;  // 128K

constexpr static u32 kPageSize       = 0x0800;  // 2k
constexpr static u32 kPageOffsetMask = 0x07FF;
constexpr static u32 kWriteSize      = 0x0008;  // 8 bytes
#else

#endif  // HAL_FLASH_MODULE_ENABLED

/********** FLASH **************************/
/*-----------------------------------------*/
/********** DMA **************************/

#ifdef HAL_DMA_MODULE_ENABLED

#if defined(STM32H750xx)

#define GET_DMA_CHANNEL(channelbase) \
    ((channelbase - DMA1_Channel1_BASE) / (DMA1_Channel2_BASE - DMA1_Channel1_BASE))
#endif

#if defined(STM32G4xx) || defined(STM32G0xx) || defined(STM32F1xx)
#define GET_DMA_CHANNEL(channelbase) \
    ((channelbase - DMA1_Channel1_BASE) / (DMA1_Channel2_BASE - DMA1_Channel1_BASE))

#endif

#endif  // HAL_FLASH_MODULE_ENABLED

/********** DMA **************************/

/********** MEMORY SECTION & ALIGNMENT  **************************/
#define CACHE_LINE_SIZE 4
#define DMA_ALIGN       __attribute__((aligned(CACHE_LINE_SIZE)))

#ifdef STM32H7xx
#define AXI_BUFFER \
    __attribute__((section(".AXI_RAM1.bss"))) __attribute__((aligned(CACHE_LINE_SIZE)))
#define RAM1_BUFFER __attribute__((section(".RAM1.bss"))) __attribute__((aligned(CACHE_LINE_SIZE)))
#define RAM2_BUFFER __attribute__((section(".RAM2.bss"))) __attribute__((aligned(CACHE_LINE_SIZE)))
#define RAM3_BUFFER __attribute__((section(".RAM3.bss"))) __attribute__((aligned(CACHE_LINE_SIZE)))
#define RAM4_BUFFER __attribute__((section(".RAM4.bss"))) __attribute__((aligned(CACHE_LINE_SIZE)))
#define BACKUP_BUFFER \
    __attribute__((section(".Backup_RAM1.bss"))) __attribute__((aligned(CACHE_LINE_SIZE)))

#define AXI_DATA \
    __attribute__((section(".AXI_RAM1.data"))) __attribute__((aligned(CACHE_LINE_SIZE)))
#define RAM1_DATA __attribute__((section(".RAM1.data"))) __attribute__((aligned(CACHE_LINE_SIZE)))
#define RAM2_DATA __attribute__((section(".RAM2.data"))) __attribute__((aligned(CACHE_LINE_SIZE)))
#define RAM3_DATA __attribute__((section(".RAM3.data"))) __attribute__((aligned(CACHE_LINE_SIZE)))
#define RAM4_DATA __attribute__((section(".RAM4.data"))) __attribute__((aligned(CACHE_LINE_SIZE)))
#define BACKUP_DATA \
    __attribute__((section(".Backup_RAM1.data"))) __attribute__((aligned(CACHE_LINE_SIZE)))
#else

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

/************ MEMORY SECTION & ALIGNMENT  **************************/

#endif
