#include "os.hpp"
#include "system.hpp"
#include "logger.hpp"
LOGGER("os")

namespace wibot {

void os::sleep(u32 ms) {
    switch (os::getContextMode()) {
        case os::ContextMode::kThread:
#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4996)
#elif defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
#elif defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#endif
            System::delayMs(ms);
#ifdef _MSC_VER
#pragma warning(pop)
#elif defined(__clang__)
#pragma clang diagnostic pop
#elif defined(__GNUC__)
#pragma GCC diagnostic pop
#endif
            break;
        case os::ContextMode::kISR:
            ASSERT(false, "Cannot call os::sleep in ISR context.");
            return;
        case os::ContextMode::kInit:
#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4996)
#elif defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
#elif defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#endif
            System::delayMs(ms);
#ifdef _MSC_VER
#pragma warning(pop)
#elif defined(__clang__)
#pragma clang diagnostic pop
#elif defined(__GNUC__)
#pragma GCC diagnostic pop
#endif
            return;
        default:
            ASSERT(false, "Unknown context mode.");
            return;
    }
};

bool os::isInThread() {
    return !arch::isIsr();
}

Mutex::Mutex(const char* name) {
    _instance = 0;
};

Mutex::~Mutex() {
    _instance = 0;
};

Result Mutex::lock(u32 timeout) {
    if (timeout == TIMEOUT_NOWAIT) {
        if (this->_instance) {
            return Result::kNoResource;
        } else {
            this->_instance = 1;
            return Result::kOk;
        }
    } else {
        if (arch::isIsr()) {
            return Result::kNotSupport;
        } else {
            uint32_t start = System::getTickMs();
            while (this->_instance) {
                if (System::getDurationMs(start) > timeout) {
                    return Result::kTimeout;
                }
            };
            this->_instance = 1;
            return Result::kOk;
        }
    }
};

void Mutex::unlock() {
    this->_instance = 0;
}

EventGroup::EventGroup(const char* name) {
    _instance = 0;
}
EventGroup::~EventGroup() {
    _instance = 0;
};

Result EventGroup::set(u32 flags) {
    auto oldFlags = this->_instance;
    auto newFlags = this->_instance | flags;
    while (!wibot::arch::syncCompareAndSwap(&this->_instance, oldFlags, newFlags)) {
        oldFlags = this->_instance;
        newFlags = this->_instance | flags;
    }
    return Result::kOk;
};

Result EventGroup::reset(u32 flags) {
    auto oldFlags = this->_instance;
    auto newFlags = this->_instance & ~flags;
    while (!wibot::arch::syncCompareAndSwap(&this->_instance, oldFlags, newFlags)) {
        oldFlags = this->_instance;
        newFlags = this->_instance & ~flags;
    }
    return Result::kOk;
};

Result EventGroup::wait(u32 flags, u32& actualFlags, EventOptions options, u32 timeout) {
    auto   ins = this->_instance;
    Result rst = Result::kOk;

    if (timeout == TIMEOUT_NOWAIT) {
        if ((options & EventOptions_WaitFlag) == EventOptions_WaitForAll) {
            rst = ((ins & flags) == flags) ? Result::kOk : Result::kNoResource;
        } else {
            rst = ((ins & flags) != 0) ? Result::kOk : Result::kNoResource;
        }
    } else {
        if (arch::isIsr()) {
            return Result::kNotSupport;
        } else {
            uint32_t start = System::getTickMs();
            if ((options & EventOptions_WaitFlag) == EventOptions_WaitForAll) {
                while ((ins & flags) != flags) {
                    if (System::getDurationMs(start) > timeout) {
                        rst = Result::kTimeout;
                        break;
                    }
                    ins = this->_instance;
                };
            } else {
                while ((ins & flags) == 0) {
                    if (System::getDurationMs(start) > timeout) {
                        rst = Result::kTimeout;
                        break;
                    }
                    ins = this->_instance;
                };
            }
        }
    }
    actualFlags = ins & flags;
    if ((rst == Result::kOk) && ((options & EventOptions_ClearFlag) == EventOptions_ClearFlag)) {
        reset(flags);
    }
    return Result::kOk;
};

MessageQueue::MessageQueue(const char* name, void* msgAddr, uint32_t msgSize, uint32_t queueSize)
    : _msgSize(msgSize),
      _instance(Slice(static_cast<uint8_t*>(msgAddr), msgSize * sizeof(uint32_t) * queueSize)) {
}

MessageQueue::~MessageQueue() {
}

Result MessageQueue::send(const void* msg, uint32_t timeout) {
    Result rst = Result::kOk;
    if (!_instance.isFull()) {
        _instance.write(static_cast<const uint8_t*>(msg), _msgSize * sizeof(uint32_t), false);
    } else {
        if (timeout == TIMEOUT_NOWAIT) {
            return Result::kNoResource;
        } else {
            if (arch::isIsr()) {
                return Result::kNotSupport;
            } else {
                uint32_t start = System::getTickMs();
                while (_instance.isFull()) {
                    if (System::getDurationMs(start) > timeout) {
                        rst = Result::kTimeout;
                        break;
                    }
                };
                if (rst == Result::kOk) {
                    _instance.write(static_cast<const uint8_t*>(msg), _msgSize * sizeof(uint32_t),
                                    false);
                }
            }
        }
    }
    return rst;
}

Result MessageQueue::receive(void* msg, uint32_t timeout) {
    Result rst = Result::kOk;
    if (!_instance.isEmpty()) {
        _instance.read(static_cast<uint8_t*>(msg), _msgSize * sizeof(uint32_t));
    } else {
        if (timeout == TIMEOUT_NOWAIT) {
            return Result::kNoResource;
        } else {
            if (arch::isIsr()) {
                return Result::kNotSupport;
            } else {
                uint32_t start = System::getTickMs();
                while (_instance.isEmpty()) {
                    if (System::getDurationMs(start) > timeout) {
                        rst = Result::kTimeout;
                        break;
                    }
                };
                if (rst == Result::kOk) {
                    _instance.read(static_cast<uint8_t*>(msg), _msgSize * sizeof(uint32_t));
                }
            }
        }
    }
    return rst;
}
Result MessageQueue::flush() {
    _instance.clear();
    return Result::kOk;
}

OsTimer::OsTimer(const char* name, Worker& worker, u32 period, u32 firstDelay) : _instance(&worker) {
    ASSERT(false, "OsTimer not support in NORTOS mode.");
}
OsTimer::~OsTimer() {
}

void OsTimer::start() {
    _instance->run();
}

void OsTimer::stop() {
    ASSERT(false, "OsTimer not support in NORTOS mode.");
}

}  // namespace wibot
