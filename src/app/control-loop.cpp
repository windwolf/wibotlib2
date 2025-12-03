#include "control-loop.hpp"
#include "async.hpp"
#include "type.hpp"

namespace wibot {
void ControlLoop::run() {
    init();

    auto ar = getLoopSignal();

    while (true) {
        doLoop();
        ar.wait(TIMEOUT_FOREVER);
    }
};

#if defined(HAL_TIM_MODULE_ENABLED)

TimerControlLoop::TimerControlLoop(TIM_HandleTypeDef& handle, u32 freqency)
    : _timer(handle), _frequency(freqency) {};

AsyncResult TimerControlLoop::getLoopSignal() {
    return _timer.start(_frequency);
};

#endif  // HAL_TIM_MODULE_ENABLED

void TriggerControlLoop::trigger() {
    _asyncSource.setDone();
};

AsyncResult TriggerControlLoop::getLoopSignal() {
    return _asyncSource.getResult(true);
};

}  // namespace wibot
