#pragma once

#include "os.hpp"
#include "async.hpp"
#include "timer.hpp"
#include "chip.hpp"

namespace wibot {

class ControlLoop : public Worker {
   public:
    void run() override;

   protected:
    virtual void init() = 0;

    virtual void doLoop() = 0;

    virtual AsyncResult getLoopSignal() = 0;

   private:
};

#if defined(HAL_TIM_MODULE_ENABLED)

class TimerControlLoop : public ControlLoop {
   public:
    TimerControlLoop(TIM_HandleTypeDef& handle, u32 freqency = 1000);

   protected:
    AsyncResult getLoopSignal() override;

   private:
    Timer _timer;
    u32   _frequency;
};

#endif  // HAL_TIM_MODULE_ENABLED

class TriggerControlLoop : public ControlLoop {
   public:
   public:
    void trigger();

   protected:
    AsyncResult getLoopSignal() override;

   private:
    AsyncSource _asyncSource;
};

}  // namespace wibot
