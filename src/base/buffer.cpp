#include "buffer.hpp"

// #include "logger.hpp"
// LOGGER("buffer")
namespace wibot {

Slice::Slice(u8* _data, u16 _size) : data(_data), size(_size) {
}

void Slice::clear() {
    memset(data, 0, size);
};

u8 Slice::getUint8(u16 index) const {
    ASSERT(index < size, "index out of range.");
    return data[index];
};

void Slice::setUint8(u16 index, u8 value) {
    ASSERT(index < size, "index out of range.");
    data[index] = value;
};
//
// void Slice::setUint8(u8 value) {
//     ASSERT(_idx < size, "index out of range.");
//     data[_idx++] = value;
// };

i8 Slice::getInt8(u16 index) const {
    ASSERT(index < size, "index out of range.");
    return *castPointer<i8>(data + index);
};

void Slice::setInt8(u16 index, i8 value) {
    ASSERT(index < size, "index out of range.");
    data[index] = *castPointer<u8>(&value);
};
// template<u16 SIZE>
//void Slice::setInt8(i8 value) {
//     ASSERT(_idx < size, "index out of range.");
//     data[_idx++] = *castPointer<u8>(&value);
// };

u16 Slice::getUint16(u16 index, Endian endian) const {
    ASSERT(index < size, "index out of range.");
    return arch::getUint16(data + index, endian);
};

void Slice::setUint16(u16 index, u16 value, Endian endian) {
    ASSERT(index < size, "index out of range.");
    arch::setUint16(data + index, value, endian);
};
// template<u16 SIZE>
//void Slice::setUint16(u16 value, Endian endian) {
//     ASSERT(_idx < size, "index out of range.");
//     arch::setUint16(data + _idx, value, endian);
//     _idx += sizeof(u16);
// };

i16 Slice::getInt16(u16 index, Endian endian) const {
    ASSERT(index < size, "index out of range.");
    auto v = arch::getUint16(data + index, endian);
    return *castPointer<i16>(&v);
};

void Slice::setInt16(u16 index, i16 value, Endian endian) {
    ASSERT(index < size, "index out of range.");
    arch::setUint16(data + index, *castPointer<u16>(&value), endian);
}
// template<u16 SIZE>
//void Slice::setInt16(i16 value, Endian endian) {
//     ASSERT(_idx < size, "index out of range.");
//     arch::setUint16(data + _idx, *castPointer<u16>(&value), endian);
//     _idx += sizeof(i16);
// }

u32 Slice::getUint32(u16 index, Endian endian) const {
    ASSERT(index < size, "index out of range.");
    return arch::getUint32(data + index, endian);
};

void Slice::setUint32(u16 index, u32 value, Endian endian) {
    ASSERT(index < size, "index out of range.");
    arch::setUint32(data + index, value, endian);
};
// template<u16 SIZE>
//void Slice::setUint32(u32 value, Endian endian) {
//     ASSERT(_idx < size, "index out of range.");
//     arch::setUint32(data + _idx, value, endian);
//     _idx += sizeof(u32);
// };

i32 Slice::getInt32(u16 index, Endian endian) const {
    ASSERT(index < size, "index out of range.");
    auto v = arch::getUint32(data + index, endian);
    return *castPointer<i32>(&v);
};

void Slice::setInt32(u16 index, i32 value, Endian endian) {
    ASSERT(index < size, "index out of range.");
    arch::setUint32(data + index, *castPointer<u32>(&value), endian);
};
// template<u16 SIZE>
//void Slice::setInt32(i32 value, Endian endian) {
//     ASSERT(_idx < size, "index out of range.");
//     arch::setUint32(data + _idx, *castPointer<u32>(&value), endian);
//     _idx += sizeof(i32);
// };

f32 Slice::getFloat(u16 index, Endian endian) const {
    ASSERT(index < size, "index out of range.");
    auto v = arch::getUint32(data + index, endian);
    return *castPointer<f32>(&v);
};

void Slice::setFloat(u16 index, f32 value, Endian endian) {
    ASSERT(index < size, "index out of range.");
    arch::setUint32(data + index, *castPointer<u32>(&value), endian);
};
// template<u16 SIZE>
//void Slice::setFloat(f32 value, Endian endian) {
//     ASSERT(_idx < size, "index out of range.");
//     arch::setUint32(data + _idx, *castPointer<u32>(&value), endian);
//     _idx += sizeof(f32);
// };

// Buffer32::Buffer32(u32 * data, u16 size)
//     : data(data), size(size) {

//       };

// Slice Buffer32::toBuffer() {
//     return Slice((u8*)data, size * 4);
// }

// u32 Buffer32::getUint32(u16 index, Endian endian) const {
//     ASSERT(index < size, "index out of range.");
//     return arch::getUint32(castPointer<u8>(data + index), endian);
// };
// void Buffer32::setUint32(u16 index, u32 value, Endian endian) {
//     ASSERT(index < size, "index out of range.");
//     arch::setUint32(castPointer<u8>(data + index), value, endian);
// };
// void Buffer32::setUint32(u32 value, Endian endian) {
//     ASSERT(_idx < size, "index out of range.");
//     arch::setUint32(castPointer<u8>(data + _idx), value, endian);
//     _idx += sizeof(u32);
// };

// i32 Buffer32::getInt32(u16 index, Endian endian) const {
//     ASSERT(index < size, "index out of range.");
//     auto v = arch::getUint32(castPointer<u8>(data + index), endian);
//     return *castPointer<i32>(&v);
// };
// void Buffer32::setInt32(u16 index, i32 value, Endian endian) {
//     ASSERT(index < size, "index out of range.");
//     arch::setUint32(castPointer<u8>(data + index),
//                     *castPointer<u32>(&value), endian);
// };
// void Buffer32::setInt32(i32 value, Endian endian) {
//     ASSERT(_idx < size, "index out of range.");
//     arch::setUint32(castPointer<u8>(data + _idx), *castPointer<u32>(&value), endian);
//     _idx += sizeof(i32);
// };

// f32 Buffer32::getFloat(u16 index, Endian endian) const {
//     ASSERT(index < size, "index out of range.");
//     auto v = arch::getUint32(castPointer<u8>(data + index), endian);
//     return *castPointer<f32>(&v);
// };
// void Buffer32::setFloat(u16 index, f32 value, Endian endian) {
//     ASSERT(index < size, "index out of range.");
//     arch::setUint32(castPointer<u8>(data + index),
//                     *castPointer<u32>(&value), endian);
// };
// void Buffer32::setFloat(f32 value, Endian endian) {
//     ASSERT(_idx < size, "index out of range.");
//     arch::setUint32(castPointer<u8>(data + _idx), *castPointer<u32>(&value), endian);
//     _idx += sizeof(f32);
// };

}  // namespace wibot
