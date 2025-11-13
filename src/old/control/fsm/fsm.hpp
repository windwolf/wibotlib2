#pragma once

#include "type.hpp"
#include "signal-group.hpp"
#include "stdint.h"

namespace wibot {
#define FSM_MAX_STATES_COUNT                (256)
#define FSM_MAX_TRANSITIONS_COUNT           (256)
#define FSM_MAX_TRANSITIONS_COUNT_PRE_STATE (8)

class FsmState;
class FsmTransition;
class Fsm;

enum class FsmStateMode : u8 {
    kInterval = 1U,
    kPoll     = 2U,
};

enum class FsmTransitionMode : u8 {
    kTimeout = 1U,
    kEvent   = 2U,
};

using FsmAction = void (*)(Fsm&, FsmState*);
using FsmGuard  = bool (*)(Fsm&, FsmState*);

struct FsmStateConfig {
    u8          stateNo;  // 0 means not registered
    const char* name;

    /**
     * @brief parent state is used to simplify transition configuration.
     * parent represents the template of the state,
     * all the children states has the transitions and actions of the parent
     * state.
     */
    u8 parentStateNo;

    FsmAction entryAction;
    FsmAction exitAction;
    FsmAction pollAction;
    u32       pollingInterval;
};

class FsmState : public LinkList {
   public:
    static FsmState create(const u8 stateNo, const char* name, const u8 parentNo,
                           const FsmAction onEnter, const FsmAction onExit,
                           const FsmAction onPolling, const u32 pollingInterval) {
        return FsmState({.stateNo         = stateNo,
                         .name            = name,
                         .parentStateNo   = parentNo,
                         .entryAction     = onEnter,
                         .exitAction      = onExit,
                         .pollAction      = onPolling,
                         .pollingInterval = pollingInterval});
    }

   public:
    explicit FsmState(FsmStateConfig&& config);

    friend class Fsm;
    friend class FsmTransition;
    u32 enterTick;
    u32 lastPollingTick;

   private:
    FsmState*      _parent;
    FsmStateConfig _config;

    void _poll(Fsm& fsm);
    void _doPoll(Fsm& fsm);
    void _enter(Fsm& fsm, FsmState* fromState);
    void _doEnter(Fsm& fsm);
    void _exit(Fsm& fsm, FsmState* toState);
    void _doExit(Fsm& fsm);
    bool _isParentOf(FsmState* state);
};

struct FsmTransitionConfig {
    u8                from;
    u8                to;
    FsmTransitionMode mode;
    union {
        SignalCheckOption signals;
        u32               timeout;
    };

    FsmGuard  guard;
    FsmAction action;
};

class FsmTransition : public LinkList {
   public:
    explicit FsmTransition(FsmTransitionConfig&& config);
    friend class Fsm;
    friend class FsmState;

   private:
    FsmState*           _from;
    FsmState*           _to;
    FsmTransitionConfig _config;

    bool _doEventCheck(Fsm& fsm, FsmState* fromState);
    bool _doTimeoutCheck(Fsm& fsm, FsmState* fromState, u32 duration);
};

class Fsm {
   public:
    friend class FsmState;
    friend class FsmTransition;
    Fsm(const char* name, u32 eventClearMask);
    void registerState(FsmState& state);
    void registerTransition(FsmTransition& transition);

    Result start(u32 stateNo, void* userData, u32 initialTick);

    void setEvent(u32 events);

    void resetEvent(u32 events);

    u32 getEvent();

    void update(u32 tick);

    void updateInc(u32 tickInc);

    FsmState* getCurrentState();

   public:
    void* userData;
    u32   lastUpdateTick;
    u32   currentTick;

   private:
    const char*    _name;
    FsmState*      _stateHead;
    FsmTransition* _transitionHead;

    SignalGroup _signals;

    FsmState* _currentState;

    void _ensureState();

    FsmState* _findStateByNo(u8 stateNo);
    void      _transitionCheck(FsmState& state);
};

}  // namespace wibot
