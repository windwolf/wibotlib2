#include "signal-group.hpp"

namespace wibot {
bool SignalGroup::check(SignalCheckOption &signalCheckFlag) {
    return (_signals & signalCheckFlag.mask) == (signalCheckFlag.value & signalCheckFlag.mask);
};
void SignalGroup::set(u32 events) {
    _signals |= events;
};
void SignalGroup::reset(u32 events) {
    _signals &= ~events;
};
void SignalGroup::clear() {
    _signals &= ~_signalClearMask;
}
u32 SignalGroup::get() {
    return _signals;
};
}  // namespace wibot
