#include "os.hpp"

#include "arch.hpp"

namespace wibot {

ContextMode getContextMode() {
    if (isInThread()) {
        return ContextMode::kThread;
    } else {
        if (arch::isIsr()) {
            return ContextMode::kISR;
        } else {
            return ContextMode::kInit;
        }
    }
};

EventGroup::EventGroup() : EventGroup("") {};

EventFlag EventGroup::fetchFlag() {
    u32 oldFlag, currentFlag;
    do {
        oldFlag     = _usedFlags;
        currentFlag = 1;
        while (oldFlag & currentFlag) {
            if (currentFlag == LAST_BIT_MASK) {
                return 0;
            }
            currentFlag <<= 1;
        }
    } while (!arch::syncCompareAndSwap(&_usedFlags, oldFlag, oldFlag | currentFlag));
    return static_cast<EventFlag>(currentFlag);
};
EventFlag EventGroup::fetchFlagPair() {
    u32 oldFlag, currentFlag1, currentFlag2;
    do {
        oldFlag      = _usedFlags;
        currentFlag1 = 1;
        while (oldFlag & currentFlag1) {
            if (currentFlag1 == LAST_BIT_MASK) {
                return 0;
            }
            currentFlag1 <<= 1;
        }
        currentFlag2 = currentFlag1 << 1;
        while (oldFlag & currentFlag2) {
            if (currentFlag2 == LAST_BIT_MASK) {
                return 0;
            }
            currentFlag2 <<= 1;
        }
    } while (
        !arch::syncCompareAndSwap(&_usedFlags, oldFlag, oldFlag | currentFlag1 | currentFlag2));
    return static_cast<EventFlag>(currentFlag1 | currentFlag2);
}

void EventGroup::releaseFlag(EventFlag flag) {
    u32 oldFlag, currentFlag;
    do {
        oldFlag     = _usedFlags;
        currentFlag = oldFlag & ~flag;
    } while (!arch::syncCompareAndSwap(&_usedFlags, oldFlag, currentFlag));
}
}  // namespace wibot
