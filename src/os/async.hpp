#ifndef WIBOTLIB_OS_ASYNC_RESULT_HPP__
#define WIBOTLIB_OS_ASYNC_RESULT_HPP__

#
#include "os.hpp"
#include "eventgrouppool.hpp"

namespace wibot {

class AsyncResult;

/**
 * @brief 异步处理结果的时间源
 * 
 */
class AsyncSource {
   public:
    enum class State : u8 {
        kPending = 0,
        kDone,
        kError,
    };

   public:
    AsyncSource();
    AsyncSource(const AsyncSource &other)            = delete;
    AsyncSource &operator=(const AsyncSource &other) = delete;
    AsyncSource(AsyncSource &&other)                 = delete;
    AsyncSource &operator=(AsyncSource &&other)      = delete;
    ~AsyncSource();

   public:
    /**
    * @brief 设置异步操作成功完成
    * 
    */
    void setDone();

    /**
     * @brief 设置异步操作失败
     * 
     * @param result 失败结果
     */
    void setError(Result result);

    /**
     * @brief 创建一个关联到此源的异步结果对象
     * 
     * @return AsyncResult 
     */
    AsyncResult getResult(bool autoReset = false);

   private:
    friend class AsyncResult;

    EventGroupPool::EventGroupStub _eventGroup;
    Result                         _result;
};

/**
 * @brief 异步处理结果的代理
 * 
 */
class AsyncResult {
   public:
    AsyncResult(const AsyncResult &other)            = delete;
    AsyncResult &operator=(const AsyncResult &other) = delete;
    AsyncResult(AsyncResult &&other) noexcept;
    AsyncResult &operator=(AsyncResult &&other) noexcept;
    ~AsyncResult();

   public:
    static AsyncResult fromResult(Result result);
    static AsyncResult fromOk();
    static AsyncResult fromError(Result result);
    static AsyncResult fromSource(AsyncSource &source, bool autoReset = false);

   public:
    /**
    * @brief 等待异步处理完成
    * 
    * @param timeout 等待超时时间.
    * @return Result 
    */
    Result wait(u32 timeout = TIMEOUT_FOREVER);

   private:
    AsyncResult(AsyncSource::State initialState);
    AsyncResult(AsyncSource &source, bool autoReset);

   private:
    AsyncSource::State _state;
    AsyncSource       *_source;
    Result             _ErrorResult;
    bool               _autoReset;
};

}  // namespace wibot

#endif  // WIBOTLIB_OS_ASYNC_RESULT_HPP__
