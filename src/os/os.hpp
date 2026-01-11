#pragma once

#include "type.hpp"
#include "os-port.hpp"
#include "buffer.hpp"

namespace wibot {

enum class ContextMode {
    kThread = 0,
    kISR    = 1,
    kInit   = 2,
};

ContextMode getContextMode();
bool        isInThread();

u32 getTickMs();

void sleep(u32 ms);

class Worker {
   public:
    virtual void run() = 0;
};

template <u16 stack_size>
class Thread {
   public:
    Thread(const char* name, Worker& worker, u32 priority,
           const ThreadConfig& config = ThreadConfig());
    ~Thread();

    void start();

   private:
    THREAD_TYPEDEF _instance;
    u8             _stack[stack_size];
};

class OsTimer {
   public:
    OsTimer(const char* name, Worker& worker, u32 period, u32 firstDelay);
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

using EventFlag = u32;

/**
 *
 */
class EventGroup {
   public:
    EventGroup();
    explicit EventGroup(const char* name);
    ~EventGroup();

   public:
    Result    set(u32 flags);
    Result    reset(u32 flags);
    Result    wait(u32 flags, u32& actual_flags, EventOptions options, u32 timeout);
    EventFlag fetchFlag();
    EventFlag fetchFlagPair();
    void      releaseFlag(EventFlag flag);

   private:
    EVENTGROUP_TYPEDEF _instance;
    u32           _usedFlags{};
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
    MessageQueue(const char* name, void* msgAddr, u32 msgSize, u32 queueSize);
    ~MessageQueue();

    Result send(const void* msg, u32 timeout);
    Result receive(void* msg, u32 timeout);
    /**
     * @brief clear the message queue
     * @return
     */
    Result flush();

   private:
    MESSAGEQUEUE_TYPEDEF _instance;
};

}  // namespace wibot

#include "port/os/os-port.tpp"
