#include "eventgrouppool.hpp"

namespace wibot {
// EventGroupPool implementation
EventGroupPool& EventGroupPool::getInstance() {
    static EventGroupPool pool;
    return pool;
}

Result EventGroupPool::fetch(EventGroupStub& eventGroup) {
    for (int i = 0; i < EVENT_POOL_SIZE; ++i) {
        auto flags = _pool[i].fetchFlagPair();
        if (flags == 0) {
            continue;
        }
        u32 firstFlag = 1;
        while (!(flags & firstFlag)) {
            firstFlag <<= 1;
        }
        eventGroup.eventGroup = &_pool[i];
        eventGroup.doneFlag   = firstFlag;
        eventGroup.errorFlag  = flags & ~firstFlag;
        return Result::kOk;
    }
    return Result::kNoResource;
}

void EventGroupPool::release(EventGroupStub& eventGroup) {
    eventGroup.eventGroup->releaseFlag(eventGroup.doneFlag | eventGroup.errorFlag);
    eventGroup.eventGroup = nullptr;
    eventGroup.errorFlag  = 0;
    eventGroup.doneFlag   = 0;
}
}  // namespace wibot
