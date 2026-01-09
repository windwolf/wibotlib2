#include "timer.hpp"

#include "peripheral.hpp"
#include "../system.hpp"
#include <utility>

#ifdef HAL_TIM_MODULE_ENABLED

namespace wibot {

Timer::Timer(TIM_HandleTypeDef& htim) : _instance(&htim), _updateEventSource() {
    HAL_TIM_RegisterCallback(&htim, HAL_TIM_PERIOD_ELAPSED_CB_ID, _onPeriodElapsedCplt);
    PeripheralManager::getInstance().registerPeripheral(this, _instance);
}

Timer::~Timer() {
    PeripheralManager::getInstance().unregisterPeripheral(this);
}

void Timer::_onPeriodElapsedCplt(TIM_HandleTypeDef* htim) {
    auto ins = (Timer*)PeripheralManager::getInstance().getPeripheral(htim);
    ins->_updateEventSource.setDone();
    if (ins->_updateEventHandler) {
        ins->_updateEventHandler();
    }
}

AsyncResult Timer::start(u32 freq) {
    auto rst = startInternal(freq);
    if (!rst.isOk()) {
        return AsyncResult::fromError(rst);
    }
    return _updateEventSource.getResult(true);
};

void Timer::start(u32 freq, TimerInteruptHandler handler) {
    _updateEventHandler = std::move(handler);

    auto rst = startInternal(freq);
    ASSERT(rst.isOk(), "Failed to start timer");
};

Result Timer::startInternal(u32 freq) {
    u32 pclkTimFreq = System::getTIMFreq(_instance->Instance);

    // 根据 PCLK 和要求的频率,计算 prescaler 和 period
    // 目标: pclkTimFreq / (prescaler * period) = freq
    // 约束: 1 <= prescaler <= 0xFFFF, 1 <= period <= 0xFFFF
    u32 totalDiv = pclkTimFreq / freq;

    ASSERT(totalDiv > 0, "Requested frequency is too high");

    // 使用整数平方根,使 prescaler 和 period 尽可能平衡
    u32 prescaler = totalDiv;
    u32 y         = (prescaler + 1) >> 1;
    while (y < prescaler) {
        prescaler = y;
        y         = (prescaler + totalDiv / prescaler) >> 1;
    }

    // 限制 prescaler 在有效范围内
    if (prescaler > 0xFFFF) {
        prescaler = 0xFFFF;
    }
    if (prescaler < 1) {
        prescaler = 1;
    }

    u32 period = totalDiv / prescaler;

    // 如果 period 超出范围,重新调整 prescaler
    if (period > 0xFFFF) {
        prescaler = (totalDiv + 0xFFFF - 1) / 0xFFFF;  // 向上取整
        period    = totalDiv / prescaler;
    }
    if (period < 1) {
        period = 1;
    }

    ASSERT(prescaler >= 1 && prescaler <= 0xFFFF && period >= 1 && period <= 0xFFFF,
           "Cannot find suitable prescaler and period for the given frequency");

    LL_TIM_SetPrescaler(_instance->Instance, prescaler - 1);
    LL_TIM_SetAutoReload(_instance->Instance, period - 1);
    LL_TIM_GenerateEvent_UPDATE(_instance->Instance);

    auto rst = HAL_TIM_Base_Start_IT(_instance);
    if (rst != HAL_OK) {
        return (Result)rst;
    }
    return Result::kOk;
};

Result Timer::stop() {
    return HAL_TIM_Base_Stop_IT(_instance);
}

}  // namespace wibot

#endif  // HAL_TIM_MODULE_ENABLED
