#pragma once

#include "type.hpp"
#include "os.hpp"

namespace wibot {
class EventGroupPool {
   public:
    struct EventGroupStub {
        EventGroup *eventGroup;
        u32         doneFlag;
        u32         errorFlag;
    };

   public:
    static EventGroupPool &getInstance();
    Result                 fetch(EventGroupStub &eventGroup);
    void                   release(EventGroupStub &eventGroup);

   private:
    EventGroup _pool[EVENT_POOL_SIZE];
};
}  // namespace wibot
