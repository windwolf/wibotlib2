#pragma once

namespace wibot {

template <typename T>
class FsmExecutor {
   public:
    FsmExecutor() : _currentState{} {
    }

    explicit FsmExecutor(T initialState) : _currentState(initialState) {
    }

    void setState(T state) {
        _currentState = state;
    }

    T getState() const {
        return _currentState;
    }

   public:
    void update() {
        T fromState = _currentState;
        T toState   = transitState(fromState);
        if (toState != fromState) {
            onStateChange(fromState, toState);
            _currentState = toState;
        }
        doStateAction(_currentState);
    };

   protected:
    virtual T    transitState(T fromState)             = 0;
    virtual void onStateChange(T fromState, T toState) = 0;
    virtual void doStateAction(T state)                = 0;

   private:
    T _currentState;
};
}  // namespace wibot

