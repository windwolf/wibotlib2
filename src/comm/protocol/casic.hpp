#pragma once

#include "type.hpp"
#include "circular-buffer.hpp"
namespace wibot::comm {
struct CasicFrameNavPv {
    u32 runTime;
    u8  posValid;
    u8  velValid;
    u8  system;
    u8  numSV;
    u8  numSVGPS;
    u8  numSVBDS;
    u8  numSVGLN;
    u8  res;
    f32 pDop;
    f64 lon;
    f64 lat;
    f32 height;
    f32 sepGeoid;
    f32 hAcc;
    f32 vAcc;
    f32 velN;
    f32 velE;
    f32 velU;
    f32 speed3D;
    f32 speed2D;
    f32 heading;
    f32 sAcc;
    f32 cAcc;
} PACKED;

bool casic_parse(u8* msg, u32 length, u16* classId, void** payload);
}  // namespace wibot::protocol
