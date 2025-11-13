#pragma once

#include "type.hpp"
namespace wibot::protocol {

#define UBX_CLASSID_NAV_PVT 0x0701

struct UbxFrameNavPvt {
    u32 iTow;
    u16 year;
    u8  month;
    u8  day;
    u8  hour;
    u8  min;
    u8  sec;
    union {
        u8 valid;
        struct {
            u8 validDate     : 1;
            u8 validTime     : 1;
            u8 fullyResolved : 1;
            u8 reserved      : 5;
        };
    } valid;
    u32 tAcc;
    i32 nano;
    u8  fixType;
    union {
        u8 flags;
        struct {
            u8 gnssFixOK : 1;
            u8 diffSoln  : 1;
            u8 psmState  : 3;
            u8 reserved  : 3;
        };
    } flags;
    u8  reserved1;
    u8  numSV;
    i32 lon;
    i32 lat;
    i32 height;
    i32 hMSL;
    u32 hAcc;
    u32 vAcc;
    i32 velN;
    i32 velE;
    i32 velD;
    i32 gSpeed;
    i32 heading;
    u32 sAcc;
    u32 headingAcc;
    u16 pDOP;
    u16 reserved2;
    u32 reserved3;
    f32 cAcc;
} PACKED;

bool ubx_parse(u8* msg, u32 length, u16* classId, void** payload);
}  // namespace wibot::protocol
