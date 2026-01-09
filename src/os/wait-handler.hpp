#pragma once

#include "os.hpp"
#include "eventgrouppool.hpp"

namespace wibot::os {

class WaitTrigger;

/**
 * @brief Implement the sender receiver async pattern.
 * Sender maintain the handler's state, then receiver wait for sender's done or
 * error signal.
 * For normally operation,
 * 1. receiver require sender for some async function, and pass the WaitHandler
 * to sender, and return immediately;
 * 2. Receiver do something else and then call wait to wait for sender's done or error
 * 3. sender do the async function, and call setDone or setError to notify receiver.
 *
 *
 */
class WaitHandler {
   public:
    WaitHandler();
    WaitHandler(const WaitHandler& other);
    ~WaitHandler();

    // void   setValue(void* value);
    // void*  getValue();
    Result reset();

    /**
     * @brief check if the handler is triggered by done or error.
     * Used for merged WaitHandler to check which WaitHandler is triggered.
     * @param handler
     * @return Result::kOk if triggered by done, Result::GeneralError if triggered,
     * Result::NoResource if not triggered.
     */
    //[[deprecated("Not fully supported yet.")]] Result triggeredFor(WaitHandler& handler);
    //[[deprecated("Not fully supported yet.")]] WaitHandler merge(const WaitHandler& other);

    Result wait(u32 timeout);

   private:
    bool _isBusy();

   private:
    friend class WaitTrigger;
    // void*       _value;
    EventGroupPool::EventGroupStub _fetchResult;
    //u32    _currentFlag;
    bool                           _isRef;
    bool                           _autoReset;
    WaitTrigger*                   _triggers;
};

class WaitTrigger : private LinkList {
   public:
    explicit WaitTrigger();
    WaitTrigger(const WaitTrigger& other);
    ~WaitTrigger();
    void attach(WaitHandler& waitHandler);
    void detach();
    bool isAttached();

    void setDone();
    void setError();

   private:
    friend class WaitHandler;
    WaitHandler* _handler;
};

}  // namespace wibot
