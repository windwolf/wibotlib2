#pragma once

#include "os/async.hpp"
#include "chip.hpp"
#include <functional>
#include "peripheral.hpp"

#ifdef HAL_TIM_MODULE_ENABLED

namespace wibot::hal {

class Timer : private PeripheralBase {
   public:
    using TimerInteruptHandler = std::function<void(void)>;

   public:
    Timer(TIM_HandleTypeDef& htim);
    ~Timer();

    /**
     * @brief 
     * 
     * @param freq 1Hz ~ 1MHz
     * @return os::AsyncResult 
     */
    os::AsyncResult start(u32 freq);
    /**
     * @brief 
     * 
     * @param freq 
     * @param handler 定时器回调 
     */
    void            start(u32 freq, TimerInteruptHandler handler);
    Result          stop();

   private:
    static void _onPeriodElapsedCplt(TIM_HandleTypeDef* htim);

    Result startInternal(u32 freq);

    TIM_HandleTypeDef*   _instance;
    os::AsyncSource      _updateEventSource;
    TimerInteruptHandler _updateEventHandler;
};

}  // namespace wibot::hal

#endif
