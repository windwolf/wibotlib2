#pragma once

#include "os/os.hpp"
#include "os/async.hpp"
#include "hal/stm32/timer.hpp"
#include "chip.hpp"
#include <functional>

namespace wibot {

class ControlLoop : public Worker {
   public:
    void run() override;

   protected:
    virtual void        init()          = 0;
    virtual void        doLoop()        = 0;
    virtual AsyncResult getLoopSignal() = 0;
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

class EventDrivenControlLoop : public ControlLoop {
   public:
    void trigger();

   protected:
    AsyncResult getLoopSignal() override;

   private:
    AsyncSource _asyncSource;
};

#if defined(HAL_TIM_MODULE_ENABLED)

template <u8 N>
class ControllLoopTrgger {
   public:
    struct ControlLoopEntry {
        EventDrivenControlLoop* loop;
        u32                     intervalTicks;
    };

   public:
    ControllLoopTrgger(TIM_HandleTypeDef& handle, u32 freqency,
                       const ControlLoopEntry (&entries)[N])
        : _timer(handle), _freqency(freqency) {
        for (u8 i = 0; i < N; i++) {
            _entries[i] = entries[i];
        }
    }

    ControllLoopTrgger(TIM_HandleTypeDef& handle, u32 freqency = 1000)
        : _timer(handle), _freqency(freqency) {
        for (u8 i = 0; i < N; i++) {
            _entries[i].loop          = nullptr;
            _entries[i].intervalTicks = 0;
        }
    }

    void start() {
        _timer.start(_freqency, std::bind(&ControllLoopTrgger::dispatch, this));
    }

    void dispatch() {
        _tick++;
        for (u8 i = 0; i < N; i++) {
            auto loop = _entries[i].loop;
            if (loop != nullptr && _entries[i].intervalTicks != 0) {
                if (_tick % _entries[i].intervalTicks == 0) {
                    loop->trigger();
                }
            }
        }
    }

   private:
    ControlLoopEntry _entries[N];
    Timer            _timer;
    u32              _freqency;
    u32              _tick = 0;
};

#endif  // HAL_TIM_MODULE_ENABLED

}  // namespace wibot
