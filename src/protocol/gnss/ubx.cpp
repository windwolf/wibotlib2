#include "ubx.hpp"

namespace wibot::protocol {
static inline bool checksum(u8* msg, u32 length) {
    u8* p    = msg + 2;
    u32 l    = length - 4;
    u8  ck_a = 0, ck_b = 0;
    for (u32 i = 0; i < l; i++) {
        ck_a += *p++;
        ck_b += ck_a;
    }
    return (ck_a == p[0]) && (ck_b == p[1]);
}

bool ubx_parse(u8* msg, u32 length, u16* classId, void** payload) {
    if (!checksum(msg, length)) {
        return false;
    }
    msg += 2;
    *classId = *(u16*)msg;
    msg += 4;
    *payload = msg;
    return true;
}
}  // namespace wibot::protocol
