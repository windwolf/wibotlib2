#pragma once

#include "stdint.h"
namespace wibot {

struct SignalCheckOption {
    u32 mask;
    u32 value;
};

class SignalGroup {
   public:
    SignalGroup(u32 signalClearMask) : _signalClearMask(signalClearMask) {};
    ;
    bool check(SignalCheckOption &signalCheckFlag);
    void set(u32 events);
    void reset(u32 events);
    void clear();
    u32  get();

   private:
    u32 _signals;
    u32 _signalClearMask;
};

}  // namespace wibot
