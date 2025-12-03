#pragma once

#include "async.hpp"
#include "chip.hpp"
#include "peripheral.hpp"

#ifdef HAL_TIM_MODULE_ENABLED

namespace wibot {

class Timer : private PeripheralBase {
   public:
    Timer(TIM_HandleTypeDef& htim);
    ~Timer();

    /**
     * @brief 
     * 
     * @param freq 1Hz ~ 1MHz
     * @return AsyncResult 
     */
    AsyncResult start(u32 freq);
    Result      stop();

   private:
    static void _onPeriodElapsedCplt(TIM_HandleTypeDef* htim);

   private:
    TIM_HandleTypeDef* _instance;
    AsyncSource        _updateEventSource;
};

}  // namespace wibot

#endif
