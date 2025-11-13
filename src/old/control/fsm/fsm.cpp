#include "fsm.hpp"
#include "logger.hpp"

namespace wibot {

LOGGER("fsm")

void FsmState::_poll(Fsm& fsm) {
    FsmState* sta = this;
    while (sta != NULL) {
        sta->_doPoll(fsm);
        sta = sta->_parent;
    }
};

void FsmState::_doPoll(Fsm& fsm) {
    if ((_config.pollAction != NULL) &&
        ((fsm.currentTick - lastPollingTick) >= _config.pollingInterval)) {
        _config.pollAction(fsm, this);
        lastPollingTick = fsm.currentTick;
    }
};

bool FsmState::_isParentOf(FsmState* state) {
    while (state != NULL) {
        if (state->_config.stateNo == _config.stateNo) {
            return true;
        }
        state = state->_parent;
    }
    return false;
}

void FsmState::_enter(Fsm& fsm, FsmState* fromState) {
    fsm._currentState = this;
    FsmState* curSta  = this;
    while (curSta != NULL && !curSta->_isParentOf(fromState)) {
        curSta->_doEnter(fsm);
        curSta = curSta->_parent;
    }

    LOG_I("%u %s.%s: entry", fsm.currentTick, (fsm._name == NULL) ? "" : fsm._name,
          (_config.name == NULL) ? "" : _config.name);
};

void FsmState::_doEnter(Fsm& fsm) {
    enterTick = fsm.currentTick;
    if (_config.entryAction != NULL) {
        _config.entryAction(fsm, this);
    }
};

void FsmState::_exit(Fsm& fsm, FsmState* toState) {
    FsmState* curSta = this;
    while (curSta != NULL && !curSta->_isParentOf(toState)) {
        curSta->_doExit(fsm);
        curSta = curSta->_parent;
    }

    LOG_I("%u %s.%s: _exit", fsm.currentTick, (fsm._name == NULL) ? "" : fsm._name,
          (_config.name == NULL) ? "" : _config.name);
};

void FsmState::_doExit(Fsm& fsm) {
    if (_config.exitAction != NULL) {
        _config.exitAction(fsm, this);
    }
}
FsmState::FsmState(FsmStateConfig&& config) {
    _config = config;
};

bool FsmTransition::_doEventCheck(Fsm& fsm, FsmState* fromState) {
    // skip transit to self.
    if (_config.to == fromState->_config.stateNo) {
        return false;
    }

    if (_config.mode == FsmTransitionMode::kEvent && fsm._signals.check(_config.signals) &&
        (_config.guard == NULL || _config.guard(fsm, fromState))) {
        fromState->_exit(fsm, _to);

        if (_config.action != NULL) {
            _config.action(fsm, fromState);
        }
        _to->_enter(fsm, fsm._currentState);
        return true;
    }
    return false;
};

bool FsmTransition::_doTimeoutCheck(Fsm& fsm, FsmState* fromState, u32 duration) {
    // skip transit to self.
    if (_config.to == fromState->_config.stateNo) {
        return false;
    }
    if (_config.mode == FsmTransitionMode::kTimeout && _config.timeout <= duration &&
        (_config.guard == NULL || _config.guard(fsm, fromState))) {
        fromState->_exit(fsm, _to);
        if (_config.action != NULL) {
            _config.action(fsm, fromState);
        }
        _to->_enter(fsm, fsm._currentState);
        return true;
    }
    return false;
}
FsmTransition::FsmTransition(FsmTransitionConfig&& config) {
    _config = config;
};

Fsm::Fsm(const char* name, u32 eventClearMask)
    : _name(name), _signals(eventClearMask) {

      };

void Fsm::_ensureState() {
    auto state = _stateHead;
    while (state != nullptr) {
        if (state->_config.parentStateNo != 0) {
            state->_parent = _findStateByNo(state->_config.parentStateNo);
            ASSERT(state->_parent != nullptr, "%s %s's parent not exsits.",
                   (_name == NULL) ? "" : _name,
                   (state->_config.name == nullptr) ? "" : state->_config.name);
        } else {
            state->_parent = nullptr;
        }
        state = static_cast<FsmState*>(state->_next);
    }

    auto transition = _transitionHead;
    while (transition != nullptr) {
        FsmState* fromState = _findStateByNo(transition->_config.from);
        FsmState* toState   = _findStateByNo(transition->_config.to);
        ASSERT(fromState != nullptr, "%s transition's from %d not exsits.",
               (_name == NULL) ? "" : _name, transition->_config.from);
        ASSERT(toState != nullptr, "%s transition's from %d not exsits.",
               (_name == NULL) ? "" : _name, transition->_config.to);
        transition->_from = fromState;
        transition->_to   = toState;
        auto state        = _stateHead;
        while (state != nullptr) {
            ASSERT(state->_config.parentStateNo == transition->_config.to,
                   "%s transition 's to state[%d] must be leaf node.", (_name == NULL) ? "" : _name,
                   transition->_config.to);

            state = static_cast<FsmState*>(state->_next);
        }

        transition = static_cast<FsmTransition*>(transition->_next);
    }
};

Result Fsm::start(u32 stateNo, void* userData, u32 initialTick) {
    _ensureState();
    currentTick     = initialTick;
    FsmState* state = _findStateByNo(stateNo);
    if (state == nullptr) {
        LOG_E("listen state: %s.%u not found.", (_name == NULL) ? "" : _name, stateNo);
        return Result::kInvalidParameter;
    }
    this->userData = userData;
    state->_enter(*this, nullptr);
    lastUpdateTick = initialTick;
    LOG_I("FSM %s started.", (_name == NULL) ? "" : _name);
    return Result::kOk;
};

// #undef FSM_TRANSITION_PREFILTER
void Fsm::_transitionCheck(FsmState& state) {
    u32       duration = (currentTick - state.enterTick);
    FsmState* curSta   = &state;
    while (curSta != NULL) {
        auto transition = _transitionHead;
        while (transition != nullptr) {
            if (transition->_from == curSta && transition->_to != &state) {
                if (transition->_doEventCheck(*this, curSta)) {
                    return;
                }
            }
            transition = static_cast<FsmTransition*>(transition->_next);
        }
        curSta = curSta->_parent;
    }

    curSta = &state;
    while (curSta != NULL) {
        auto transition = _transitionHead;
        while (transition != nullptr) {
            if (transition->_from == curSta && transition->_to != &state) {
                if (transition->_doTimeoutCheck(*this, curSta, duration)) {
                    return;
                }
            }
            transition = static_cast<FsmTransition*>(transition->_next);
        }

        curSta = curSta->_parent;
    }
};

void Fsm::setEvent(u32 events) {
    this->_signals.set(events);
};

void Fsm::resetEvent(u32 events) {
    this->_signals.reset(events);
};

void Fsm::update(u32 tick) {
    currentTick     = tick;
    FsmState* state = _currentState;
    if (state == nullptr) {
        return;
    }

    state->_poll(*this);
    _transitionCheck(*state);
    _signals.clear();

    lastUpdateTick = tick;
};

void Fsm::updateInc(u32 tickInc) {
    update(lastUpdateTick + tickInc);
};

FsmState* Fsm::_findStateByNo(u8 stateNo) {
    auto state = _stateHead;
    while (state != nullptr) {
        if (state->_config.stateNo == stateNo) {
            return state;
        }
        state = static_cast<FsmState*>(state->_next);
    }
    return nullptr;
}
u32 Fsm::getEvent() {
    return _signals.get();
}
FsmState* Fsm::getCurrentState() {
    return _currentState;
}
void Fsm::registerState(FsmState& state) {
    if (_stateHead == nullptr) {
        _stateHead = &state;
    } else {
        _stateHead->append(&state);
    }
}
void Fsm::registerTransition(FsmTransition& transition) {
    if (_transitionHead == nullptr) {
        _transitionHead = &transition;
    } else {
        _transitionHead->append(&transition);
    }
};
}  // namespace wibot
