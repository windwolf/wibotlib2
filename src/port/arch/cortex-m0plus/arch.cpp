//
// Created by zhouj on 2023/1/28.
//

#include "arch.hpp"
#ifdef __cplusplus
extern "C" {
#endif
#include "cmsis_compiler.h"
#ifdef __cplusplus
}
#endif
namespace wibot::arch {

bool isIsr() {
    return __get_IPSR() != 0;
}

bool syncCompareAndSwap(volatile u32* ptr, u32 oldValue, u32 newValue) {
    //        u32 oldval, res;
    //        __asm__ __volatile__("@ SyncCompareAndSwap\n"
    //                             "ldrex  %1, [%2]\n"
    //                             "mov    %0, #0\n"
    //                             "teq    %1, %3\n"
    //                             "strexeq %0, %4, [%2]\n"
    //            : "=&r" (res), "=&r" (oldval)
    //            : "r" (ptr), "Ir" (old_value), "r" (new_value)
    //            : "cc");
    //        return res;
    auto prim = __get_PRIMASK();
    __disable_irq();
    if (*ptr == oldValue) {
        *ptr = newValue;
        if (!prim) {
            __enable_irq();
        }
        return true;
    } else {
        if (!prim) {
            __enable_irq();
        }
        return false;
    }
};

void enterCritical() {
    __disable_irq();
};
void exitCritical() {
    __enable_irq();
};

u32 getUint32(u8* data, Endian endian) {
    if (endian == Endian::kLittle) {
        return *static_cast<u32*>(static_cast<void*>(data));
    } else {
        return (data[0] << 24) + (data[1] << 16) + (data[2] << 8) + data[3];
    }
};

void setUint32(u8* data, u32 value, Endian endian) {
    if (endian == Endian::kLittle) {
        *static_cast<u32*>(static_cast<void*>(data)) = value;
    } else {
        data[0] = (value >> 24) & 0xFF;
        data[1] = (value >> 16) & 0xFF;
        data[2] = (value >> 8) & 0xFF;
        data[3] = value & 0xFF;
    }
};

u16 getUint16(u8* data, Endian endian) {
    if (endian == Endian::kLittle) {
        return *static_cast<u16*>(static_cast<void*>(data));
    } else {
        return (data[0] << 8) + data[1];
    }
};
void setUint16(u8* data, u16 value, Endian endian) {
    if (endian == Endian::kLittle) {
        *static_cast<u16*>(static_cast<void*>(data)) = value;
    } else {
        data[0] = (value >> 8) & 0xFF;
        data[1] = value & 0xFF;
    }
}
}  // namespace wibot::arch
