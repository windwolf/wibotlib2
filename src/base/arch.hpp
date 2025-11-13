#pragma once

//
// Created by zhouj on 2023/1/28.
//

#include "type.hpp"

namespace wibot::arch {

bool isIsr();
bool syncCompareAndSwap(volatile u32* ptr, u32 oldValue, u32 newValue);
u32  getUint32(u8* data, Endian endian);
void setUint32(u8* data, u32 value, Endian endian);
u16  getUint16(u8* data, Endian endian);
void setUint16(u8* data, u16 value, Endian endian);

void enterCritical();
void exitCritical();

}  // namespace wibot::arch
