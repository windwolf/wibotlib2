#pragma once

#include "type.hpp"
#include "os-port.hpp"
#include "buffer.hpp"

namespace wibot::os {

enum class ContextMode {
    kThread = 0,
    kISR    = 1,
    kInit   = 2,
};

ContextMode getContextMode();
bool        isInThread();

uint32_t getTickMs();

void sleep(uint32_t ms);


class Worker {
   public:
    virtual void run() = 0;
};

template <u16 stack_size>
class Thread {
   public:
    Thread(const char* name, Worker& worker, uint32_t priority,
           const ThreadConfig& config = ThreadConfig());
    ~Thread();

    void start();

   private:
    THREAD_TYPEDEF _instance;
    u8             _stack[stack_size];
};

class OsTimer {
   public:
    OsTimer(const char* name, Worker& worker, uint32_t period, uint32_t firstDelay);
    ~OsTimer();

    void start();
    void stop();

   private:
    TIMER_TYPEDEF _instance;
};

class Mutex {
   public:
    explicit Mutex(const char* name);
    ~Mutex();

   public:
    Result lock(uint32_t timeout);
    void   unlock();

   private:
    MUTEX_TYPEDEF _instance{};
};

using EventOptions = uint8_t;

constexpr EventOptions EventOptions_WaitFlag   = 0x02;
constexpr EventOptions EventOptions_WaitForAny = 0x00;
constexpr EventOptions EventOptions_WaitForAll = EventOptions_WaitFlag;
constexpr EventOptions EventOptions_ClearFlag  = 0x01;
constexpr EventOptions EventOptions_NoClear    = 0x00;
constexpr EventOptions EventOptions_Clear      = EventOptions_ClearFlag;

using EventFlag = uint32_t;

/**
 *
 */
class EventGroup {
   public:
    EventGroup();
    explicit EventGroup(const char* name);
    ~EventGroup();

   public:
    Result    set(uint32_t flags);
    Result    reset(uint32_t flags);
    Result    wait(uint32_t flags, uint32_t& actual_flags, EventOptions options, uint32_t timeout);
    EventFlag fetchFlag();
    EventFlag fetchFlagPair();
    void      releaseFlag(EventFlag flag);

   private:
    EVENTGROUP_TYPEDEF _instance;
    uint32_t           _usedFlags{};
};

class MessageQueue {
   public:
    /**
     *
     * @param name
     * @param msgAddr message queue buffer address
     * @param msgSize message size in WORD
     * @param queueSize queue size in message
     */
    MessageQueue(const char* name, void* msgAddr, uint32_t msgSize, uint32_t queueSize);
    ~MessageQueue();

    Result send(const void* msg, uint32_t timeout);
    Result receive(void* msg, uint32_t timeout);
    /**
     * @brief clear the message queue
     * @return
     */
    Result flush();

   private:
    MESSAGEQUEUE_TYPEDEF _instance;
};

}  // namespace wibot
