#include "control-loop.hpp"

namespace wibot::app {
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

os::AsyncResult TimerControlLoop::getLoopSignal() {
    return _timer.start(_frequency);
};

#endif  // HAL_TIM_MODULE_ENABLED

void EventDrivenControlLoop::trigger() {
    _asyncSource.setDone();
};

os::AsyncResult EventDrivenControlLoop::getLoopSignal() {
    return _asyncSource.getResult(true);
};

}  // namespace wibot::app
