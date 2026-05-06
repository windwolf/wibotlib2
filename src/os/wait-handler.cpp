
#include "wait-handler.hpp"

#include "arch.hpp"

#include "logger.hpp"
LOGGER("wait-handler")

namespace wibot {

WaitHandler::WaitHandler() : _isRef(false), _autoReset(true), _triggers(nullptr) {
    ASSERT(!arch::isIsr(), "Can not create wait handler in ISR");
    auto rst = EventGroupPool::getInstance().fetch(_fetchResult);
    ASSERT(rst == Result::kOk, "No event group available");
};

//WaitHandler::WaitHandler(WaitHandler::FetchResult& result) {
//    fetchResult_ = result;
//    isMerge_     = true;
//};
WaitHandler::WaitHandler(const WaitHandler& other) {
    _fetchResult = other._fetchResult;
    _isRef       = true;
    _autoReset   = other._autoReset;
    _triggers    = nullptr;
}

WaitHandler::~WaitHandler() {
    arch::enterCritical();
    WaitTrigger* curr = this->_triggers;
    while (curr != nullptr) {
        curr->_handler = nullptr;
        curr           = static_cast<WaitTrigger*>(curr->_next);
    }
    arch::exitCritical();
    if (!_isRef) {
        EventGroupPool::getInstance().release(_fetchResult);
    }
}

//void WaitHandler::setValue(void* value) {
//    _value = value;
//};
//
//void* WaitHandler::getValue() {
//    return _value;
//};
//Result WaitHandler::triggeredFor(WaitHandler& handler) {
//    if (_fetchResult.eventGroup != handler._fetchResult.eventGroup) {
//        return Result::kNoResource;
//    }
//    if (_currentFlag & handler._fetchResult.doneFlag) {
//        this->_value  = handler._value;
//        return Result::kOk;
//    }
//    if (_currentFlag & handler._fetchResult.errorFlag) {
//        this->_value  = handler._value;
//        return Result::kError;
//    }
//    return Result::kNoResource;
//}

Result WaitHandler::wait(u32 timeout) {
    u32  events;
    auto rst = _fetchResult.eventGroup->wait(
        _fetchResult.doneFlag | _fetchResult.errorFlag, events,
        EventOptions_WaitForAny | (_autoReset ? EventOptions_Clear : EventOptions_NoClear),
        timeout);

    if (rst == Result::kOk) {
        if (events & _fetchResult.errorFlag) {
            return Result::kError;
        } else if (events & _fetchResult.doneFlag) {
            return Result::kOk;
        } else {
            // No possible to reach here
            return Result::kError;
        }
    } else {
        return Result::kTimeout;
    }
};

bool WaitHandler::_isBusy() {
    u32  events;
    auto rst = _fetchResult.eventGroup->wait(_fetchResult.doneFlag | _fetchResult.errorFlag, events,
                                             EventOptions_WaitForAny | EventOptions_NoClear, 0);
    if (rst == Result::kOk) {
        return false;
    } else {
        return true;
    }
};

Result WaitHandler::reset() {
    if (_isBusy()) {
        return Result::kBusy;
    }
    _fetchResult.eventGroup->reset(_fetchResult.doneFlag | _fetchResult.errorFlag);
    return Result::kOk;
};

void WaitTrigger::setDone() {
    // It is safe. Because _handler's destroy only happens on MAIN thread (not on ISR), so following process will not be interrupted by handler destruct process.
    if (_handler != nullptr) {
        _handler->_fetchResult.eventGroup->set(_handler->_fetchResult.doneFlag);
    }
};

void WaitTrigger::setError() {
    // It is safe. Because _handler's destroy only happens on MAIN thread (not on ISR), so following process will not be interrupted by handler destruct process.
    if (_handler != nullptr) {
        _handler->_fetchResult.eventGroup->set(_handler->_fetchResult.errorFlag);
    }
};
WaitTrigger::WaitTrigger() : _handler(nullptr) {};

WaitTrigger::WaitTrigger(const WaitTrigger& other) {
    attach(*other._handler);
};
WaitTrigger::~WaitTrigger() {
    detach();
}
void WaitTrigger::attach(WaitHandler& waitHandler) {
    if (_handler != nullptr) {
        if (_handler->_triggers == this) {
            _handler->_triggers = static_cast<WaitTrigger*>(_handler->_triggers->_next);
        } else {
            _handler->_triggers->remove(this);
        }
    }
    _handler = &waitHandler;
    if (_handler->_triggers == nullptr) {
        _handler->_triggers = this;
    } else {
        _handler->_triggers->append(this);
    }
}

void WaitTrigger::detach() {
    if (_handler != nullptr) {
        if (_handler->_triggers == this) {
            _handler->_triggers = static_cast<WaitTrigger*>(_handler->_triggers->_next);
        } else {
            _handler->_triggers->remove(this);
        }
    }
    _handler = nullptr;
}

bool WaitTrigger::isAttached() {
    return _handler != nullptr;
}

}  // namespace wibot
