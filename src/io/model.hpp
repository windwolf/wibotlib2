#pragma once

#include "async.hpp"
#include "type.hpp"

namespace wibot {

/**
 * @brief 数据处理管道的基础接口
 * 
 * SyncPipeline 提供了流式数据处理的核心抽象，支持管道嵌套和链式组合。
 * 所有数据源、滤波器、映射器、分桶器等都实现此接口。
 * 
 * @tparam T 管道处理的数据类型
 */
template <typename TCHANNEL, typename TALL = TCHANNEL *>
class SyncPipeline {
   public:
    /**
     * @brief 更新管道状态
     * 
     * 触发数据的流式处理，从上游获取数据并进行处理。
     * 对于数据源管道，通常从硬件或缓存中读取数据。
     * 对于处理管道，从上游管道获取数据并应用处理逻辑。
     */
    virtual void update() = 0;

    /**
     * @brief 获取管道输出值
     * 
     * @param channel 通道索引，默认为0（单通道）
     * @return T 处理后的输出值
     */
    virtual TCHANNEL getValue(u8 channel) const = 0;
    virtual TALL     getValues() const          = 0;

    /**
     * @brief 重置管道状态
     * 
     * 清除所有内部缓存和历史状态，将管道重置到初始状态。
     */
    virtual void reset() = 0;
};

/**
 * @brief 周期性异步事件源
 * 
 */
class AsyncEventSource {
   public:
    /**
    * @brief 订阅异步事件, 返回可重复触发的异步结果
    * 
    * @return AsyncResult 
    */
    virtual AsyncResult subscribe() = 0;
    virtual Result      start()     = 0;
    virtual Result      stop()      = 0;
};

template <typename T>
class AsyncReader {
   public:
    /**
      * @brief 读取数据到缓冲区, 直到填满data缓冲区为止才发出完成通知.
      * 
      * @param data 
      * @return AsyncResult 
      */
    virtual AsyncResult read(T &data) = 0;
};

template <typename T>
class AsyncWriter {
   public:
    /**
     * @brief 写入数据, 直到完成才发出完成通知.
     * 
     * @note `write`不能并行使用
     * @note 调用者需确保在异步通知到达前, data缓冲区不会被修改或释放.
     * @param data 
     * @return AsyncResult 
     */
    virtual AsyncResult write(const T &data) = 0;
};

template <typename T>
class SyncReader {
   public:
    /**
      * @brief 读取数据到缓冲区, 直到填满data缓冲区为止才发出完成通知.
      * 
      * @param data 
      * @return AsyncResult 
      */
    virtual Result read(T &data, u32 timeout = TIMEOUT_FOREVER) = 0;
};

template <typename T>
class SyncWriter {
   public:
    /**
      * @brief 读取数据到缓冲区, 直到填满data缓冲区为止才发出完成通知.
      * 
      * @param data 
      * @return AsyncResult 
      */
    virtual Result write(const T &data, u32 timeout = TIMEOUT_FOREVER) = 0;
};

template <typename T>
class AsyncRegisterClient {
   public:
    virtual AsyncResult readReg(u16 regAddr, T &data)        = 0;
    virtual AsyncResult writeReg(u16 regAddr, const T &data) = 0;
};

}  // namespace wibot