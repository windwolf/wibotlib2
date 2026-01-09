
#include "async.hpp"

#include "arch.hpp"

namespace wibot::os {
AsyncSource::AsyncSource() : _result(Result::kOk) {
    Result rst = EventGroupPool::getInstance().fetch(_eventGroup);
    ASSERT(rst.isOk(), "Failed to fetch event group from pool");
}

AsyncSource::~AsyncSource() {
    if (_eventGroup.eventGroup != nullptr) {
        EventGroupPool::getInstance().release(_eventGroup);
    }
}

void AsyncSource::setDone() {
    _result = Result::kOk;
    if (_eventGroup.eventGroup != nullptr) {
        _eventGroup.eventGroup->set(_eventGroup.doneFlag);
    }
}

void AsyncSource::setError(Result result) {
    _result = result;
    if (_eventGroup.eventGroup != nullptr) {
        _eventGroup.eventGroup->set(_eventGroup.errorFlag);
    }
}

AsyncResult AsyncSource::getResult(bool autoReset) {
    return AsyncResult::fromSource(*this, autoReset);
}

// AsyncResult implementation
AsyncResult::AsyncResult(AsyncSource& source, bool autoReset)
    : _state(AsyncSource::State::kPending), _source(&source), _autoReset(autoReset) {
}

AsyncResult::AsyncResult(AsyncSource::State initialState)
    : _state(initialState), _source(nullptr), _autoReset(false) {
}

AsyncResult::AsyncResult(AsyncResult&& other) noexcept
    : _state(other._state), _source(other._source), _autoReset(other._autoReset) {
    other._source = nullptr;
}

AsyncResult& AsyncResult::operator=(AsyncResult&& other) noexcept {
    if (this != &other) {
        _state        = other._state;
        _source       = other._source;
        _autoReset    = other._autoReset;
        other._source = nullptr;
    }
    return *this;
}

AsyncResult::~AsyncResult() {
    // AsyncResult doesn't own the AsyncSource, so no cleanup needed
}

Result AsyncResult::wait(u32 timeout) {
    // If there's no source, return the current state
    if (_source == nullptr) {
        switch (_state) {
            case AsyncSource::State::kDone:
                return Result::kOk;
            case AsyncSource::State::kError:
                return _ErrorResult;
            case AsyncSource::State::kPending:
                return Result::kError;
            default:
                return Result::kError;
        }
    }

    // If already completed, return immediately
    if (_state == AsyncSource::State::kDone) {
        return Result::kOk;
    }

    // If already in error, return error immediately
    if (_state == AsyncSource::State::kError) {
        return _ErrorResult;
    }

    auto stub = _source->_eventGroup;
    ASSERT(stub.eventGroup != nullptr, "event group must not be null");

    // Wait for completion

    u32 actualFlags;
    u32 waitFlags = stub.doneFlag | stub.errorFlag;

    Result waitResult = stub.eventGroup->wait(
        waitFlags, actualFlags, EventOptions_WaitForAny | EventOptions_Clear, timeout);
    if (waitResult.isOk()) {
        // Check which flag was set
        if (actualFlags & stub.doneFlag) {
            if (_autoReset) {
                _state = AsyncSource::State::kPending;
            } else {
                _state = AsyncSource::State::kDone;
            }
            return Result::kOk;
        }
        if (actualFlags & stub.errorFlag) {
            if (_autoReset) {
                _state = AsyncSource::State::kPending;
            } else {
                _state = AsyncSource::State::kError;
            }
            _ErrorResult = _source->_result;
            return _ErrorResult;
        }
        ASSERT(false, "actualFlags must contain either doneFlag or errorFlag");

    } else {
        _state = AsyncSource::State::kPending;
        return Result::kTimeout;
    }
}

AsyncResult AsyncResult::fromResult(Result result) {
    if (result.isOk()) {
        return fromOk();
    } else {
        return fromError(result);
    }
};

AsyncResult AsyncResult::fromOk() {
    return AsyncResult(AsyncSource::State::kDone);
};
AsyncResult AsyncResult::fromError(Result result) {
    AsyncResult asyncResult(AsyncSource::State::kError);
    asyncResult._ErrorResult = result;
    return asyncResult;
};

AsyncResult AsyncResult::fromSource(AsyncSource& source, bool autoReset) {
    return AsyncResult(source, autoReset);
};

}  // namespace wibot::os
