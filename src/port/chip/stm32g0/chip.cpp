//
// Created by zhouj on 2023/9/12.
//
#include "stm32g031xx.h"
#include "system.hpp"
namespace wibot {

__STATIC_INLINE u32 LL_SYSTICK_IsActiveCounterFlag() {
    return ((SysTick->CTRL & SysTick_CTRL_COUNTFLAG_Msk) == (SysTick_CTRL_COUNTFLAG_Msk));
}

u32 System::getSysClockFreq() {
    return HAL_RCC_GetSysClockFreq();
}

u32 System::getHCLKFreq() {
    return HAL_RCC_GetHCLKFreq();
}

u32 System::getPCLK1Freq() {
    return HAL_RCC_GetPCLK1Freq();
}

// u32 System::getPCLK2Freq() {
//     return HAL_RCC_GetPCLK2Freq();
// }

constexpr u8 GetTimerAPBIndex(TIM_TypeDef* instance) {
    return 1;
}

u32 System::getPCLK1TimFreq() {
    u32 pclk1 = getPCLK1Freq();

    if (READ_BIT(RCC->CFGR, RCC_CFGR_PPRE_2) == 0x00) {
        return pclk1;
    } else {
        return pclk1 * 2;
    }
}

// u32 System::getPCLK2TimFreq() {
//     u32 pclk2 = getPCLK2Freq();
//     if (READ_BIT(RCC->CFGR, RCC_CFGR_PPRE2_Msk) & RCC_CFGR_PPRE2_DIV1) {
//         return pclk2;
//     } else {
//         return pclk2 * 2;
//     }
// }

u32 System::getTIMFreq(TIM_TypeDef* tim) {
    auto apbIdx = GetTimerAPBIndex(tim);
    ASSERT(apbIdx != 0, "Invalid TIM instance");
    return getPCLK1TimFreq();
}

u32 System::getTickMs() {
    return HAL_GetTick();
}
u64 System::getTickUs() {
    /* Ensure COUNTFLAG is reset by reading SysTick control and status register_instance */
    LL_SYSTICK_IsActiveCounterFlag();
    __IO u32 m   = HAL_GetTick();
    __IO u32 tms = SysTick->LOAD + 1;
    __IO u32 u   = tms - SysTick->VAL;
    if (LL_SYSTICK_IsActiveCounterFlag()) {
        m = HAL_GetTick();
        u = tms - SysTick->VAL;
    }
    return (m * 1000 + (u * 1000) / tms);
}
u64 System::getTickNs() {
    /* Ensure COUNTFLAG is reset by reading SysTick control and status register_instance */
    LL_SYSTICK_IsActiveCounterFlag();
    __IO u32 m   = HAL_GetTick();
    __IO u32 tms = SysTick->LOAD + 1;
    __IO u32 u   = tms - SysTick->VAL;
    if (LL_SYSTICK_IsActiveCounterFlag()) {
        m = HAL_GetTick();
        u = tms - SysTick->VAL;
    }
    return (m * 1000000 + (u * 1000000) / tms);
}
void System::delayMs(u32 ms) {
    return HAL_Delay(ms);
}
void System::delayUs(u32 us) {
    static const u32 f     = HAL_RCC_GetSysClockFreq() / 8 / 1000000U;
    __IO u32         delay = us * f;
    do {
        __NOP();
        delay = delay - 1;
    } while (delay);
}
}  // namespace wibot
