#pragma once

#include "arch.hpp"
#include "stdint.h"
#include "type.hpp"
#include <array>

namespace wibot {

struct Slice {
   public:
    Slice(u8 &data) : Slice(&data, 1) {};
    Slice(u16 &data) : Slice((u8 *)&data, 2) {};
    Slice(u32 &data) : Slice((u8 *)&data, 4) {};
    Slice(u64 &data) : Slice((u8 *)&data, 8) {};
    Slice(i8 &data) : Slice((u8 *)&data, 1) {};
    Slice(i16 &data) : Slice((u8 *)&data, 2) {};
    Slice(i32 &data) : Slice((u8 *)&data, 4) {};
    Slice(i64 &data) : Slice((u8 *)&data, 8) {};
    Slice(float &data) : Slice((u8 *)&data, 4) {};
    Slice(u8 *data, u16 size);

    void clear();

    // void resetIndex();

    u8   getUint8(u16 index) const;
    void setUint8(u16 index, u8 value) const;
    //void setUint8(u8 value);

    i8   getInt8(u16 index) const;
    void setInt8(u16 index, i8 value) const;
    //void setInt8(i8 value);

    u16  getUint16(u16 index, Endian endian = Endian::kBig) const;
    void setUint16(u16 index, u16 value, Endian endian = Endian::kBig) const;
    //void setUint16(u16 value, Endian endian = Endian::kBig);

    i16  getInt16(u16 index, Endian endian = Endian::kBig) const;
    void setInt16(u16 index, i16 value, Endian endian = Endian::kBig) const;
    //void setInt16(i16 value, Endian endian = Endian::kBig);

    u32  getUint32(u16 index, Endian endian = Endian::kBig) const;
    void setUint32(u16 index, u32 value, Endian endian = Endian::kBig) const;
    //void setUint32(u32 value, Endian endian = Endian::kBig);

    i32  getInt32(u16 index, Endian endian = Endian::kBig) const;
    void setInt32(u16 index, i32 value, Endian endian = Endian::kBig) const;
    //void setInt32(i32 value, Endian endian = Endian::kBig);

    f32  getFloat(u16 index, Endian endian = Endian::kBig) const;
    void setFloat(u16 index, f32 value, Endian endian = Endian::kBig) const;
    //void setFloat(f32 value, Endian endian = Endian::kBig);

    /**
     * @brief 数据指针
     * 
     */
    u8 *data;

    /**
     * @brief 数据长度（字节）
     * 
     */
    u16 size;
};

template <u16 CAP, typename T = u8>
struct Buffer {
    u16 size = CAP;
    T   data[CAP];

    constexpr static u16 cap = CAP;

    operator Slice() {
        return Slice(data, sizeof(T) * size);
    };

    Slice toSlice(u16 length, u16 offset = 0) {
        return Slice(&data[offset], length);
    };

    T &operator[](u32 index) {
        return data[index];
    };

    const T &operator[](u32 index) const {
        return data[index];
    };
};

}  // namespace wibot
