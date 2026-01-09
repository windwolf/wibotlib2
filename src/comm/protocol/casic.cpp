#include "casic.hpp"

namespace wibot::comm {
static inline bool checksum(u8* msg, u32 length) {
    u32* p        = (u32*)(msg + 2);
    u32  l        = (length - 6) / 4;
    u32  checksum = 0;
    for (u32 i = 0; i < l; i++) {
        checksum += *p++;
    }
    return checksum == *p;
}

bool casic_parse(u8* msg, u32 length, u16* classId, void** payload) {
    if (!checksum(msg, length)) {
        return false;
    }
    msg += 4;
    *classId = *(u16*)msg;
    msg += 2;
    *payload = msg;
    return true;
};
}  // namespace wibot::protocol
