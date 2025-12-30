#include "tx_port.h"
#include "os.hpp"
#include "system.hpp"
#include "tx_api.h"

#include "logger.hpp"
LOGGER("os")

#ifdef __cplusplus
extern "C" {
#endif
void runStub(ULONG instance) {
    static_cast<wibot::Worker*>(reinterpret_cast<void*>(instance))->run();
};
#ifdef __cplusplus
}
#endif

namespace wibot {

void os::sleep(u32 ms) {
    switch (os::getContextMode()) {
        case os::ContextMode::kThread:
            tx_thread_sleep(ms);
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
    return tx_thread_identify() != TX_NULL;
}

OsTimer::OsTimer(const char* name, Worker& worker, u32 period, u32 firstDelay) {
    auto rst = tx_timer_create(&_instance, const_cast<CHAR*>(name), runStub,
                               reinterpret_cast<ULONG>(&worker), firstDelay, period, TX_NO_ACTIVATE);
    ASSERT(rst == TX_SUCCESS, "create OsTimer failed.")
};

OsTimer::~OsTimer() {
    tx_timer_delete(&_instance);
};

void OsTimer::start() {
    tx_timer_activate(&_instance);
};

void OsTimer::stop() {
    tx_timer_deactivate(&_instance);
}

// template<u32 stack_size>
// Thread<stack_size>::Thread(const char* name, void (* func)(void*), void* arg, u32 priority)
//{
//     tx_thread_create(&_instance, const_cast<CHAR*>(name), func, arg, _stack, stack_size,
//     priority, priority, TX_NO_TIME_SLICE, TX_AUTO_START);
//
// };
Mutex::Mutex(const char* name) {
    auto rst = tx_mutex_create(&(_instance), const_cast<CHAR*>(name), 0);
    ASSERT(rst == TX_SUCCESS, "create mutex failed.")
};
Mutex::~Mutex() {
    tx_mutex_delete(&(_instance));
};

Result Mutex::lock(u32 timeout) {
    auto rst = tx_mutex_get(&(_instance), timeout);
    return (rst == TX_SUCCESS) ? Result::kOk : Result::kError;
};

void Mutex::unlock() {
    tx_mutex_put(&(_instance));
}
EventGroup::EventGroup(const char* name) {
    auto rst = tx_event_flags_create(&(_instance), const_cast<CHAR*>(name));
    ASSERT(rst == TX_SUCCESS, "create eventflags failed.")
};

EventGroup::~EventGroup() {
    tx_event_flags_delete(&(_instance));
};

Result EventGroup::set(u32 flags) {
    return (tx_event_flags_set(&(_instance), flags, TX_OR) == TX_SUCCESS) ? Result::kOk
                                                                          : Result::kError;
};

Result EventGroup::reset(u32 flags) {
    return (tx_event_flags_set(&(_instance), flags, TX_AND) == TX_SUCCESS) ? Result::kOk
                                                                           : Result::kError;
};

Result EventGroup::wait(u32 flags, u32& actualFlags, EventOptions options, u32 timeout) {
    // TODO: handler TX_OPTION
    return (tx_event_flags_get(&(_instance), flags, options, (ULONG*)&actualFlags, timeout) ==
            TX_SUCCESS)
               ? Result::kOk
               : Result::kError;
};
MessageQueue::MessageQueue(const char* name, void* msgAddr, u32 msgSize, u32 queueSize) {
    auto rst = tx_queue_create(&(_instance), const_cast<CHAR*>(name), msgSize, msgAddr, queueSize);
    ASSERT(rst == TX_SUCCESS, "create messagequeue failed.")
};

MessageQueue::~MessageQueue() {
    tx_queue_delete(&(_instance));
};

Result MessageQueue::send(const void* msg, u32 timeout) {
    return (tx_queue_send(&(_instance), const_cast<void*>(msg), timeout) == TX_SUCCESS)
               ? Result::kOk
               : Result::kError;
};

Result MessageQueue::receive(void* msg, u32 timeout) {
    return (tx_queue_receive(&(_instance), const_cast<void*>(msg), timeout) == TX_SUCCESS)
               ? Result::kOk
               : Result::kError;
};
Result MessageQueue::flush() {
    return (tx_queue_flush(&(_instance)) == TX_SUCCESS) ? Result::kOk : Result::kError;
};

}  // namespace wibot
