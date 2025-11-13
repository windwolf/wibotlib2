//
// Created by zhouj on 2023/4/8.
//

#include "circular-buffer.hpp"

#include <algorithm>

#include "logger.hpp"
LOGGER("cb")

#define WrapLogicIndex(a) ((a) & ((_capacity << 1) - 1))
#define WrapMemIndex(a)   ((a) & (_capacity - 1))

namespace wibot {

template <typename TE>
u32 CircularBuffer<TE>::peek(TE *data, u32 start, u32 length) {
    auto size = getSize();
    if (start >= size) {
        return 0;
    }
    if (length > size - start) {
        length = size - start;
    }
    auto readMemIndex         = WrapMemIndex(_read + start);
    auto roomFromReadToBotton = _capacity - readMemIndex;
    if (length <= roomFromReadToBotton) {
        memcpy(data, _buffer + readMemIndex, length * sizeof(TE));
    } else {
        memcpy(data, _buffer + readMemIndex, roomFromReadToBotton * sizeof(TE));
        memcpy(data + roomFromReadToBotton, _buffer, (length - roomFromReadToBotton) * sizeof(TE));
    }
    return length;
}

template <typename TE>
u32 CircularBuffer<TE>::read(TE *data, u32 length) {
    auto size = getSize();
    if (length > size) {
        length = size;
    }
    auto readMemIndex         = WrapMemIndex(_read);
    auto roomFromReadToBotton = _capacity - readMemIndex;
    if (length <= roomFromReadToBotton) {
        memcpy(data, _buffer + readMemIndex, length * sizeof(TE));
    } else {
        memcpy(data, _buffer + readMemIndex, roomFromReadToBotton * sizeof(TE));
        memcpy(data + roomFromReadToBotton, _buffer, (length - roomFromReadToBotton) * sizeof(TE));
    }
    _read = WrapLogicIndex(_read + length);
    return length;
}

template <typename TE>
bool CircularBuffer<TE>::readVirtual(u32 length) {
    auto size     = getSize();
    auto overflow = false;
    if (length > size) {
        overflow = true;
        length   = size;
    }
    _read = WrapLogicIndex(_read + length);
    if (overflow) {
        _write = _read;
    }
    return overflow;
}
template <typename TE>
bool CircularBuffer<TE>::writeVirtual(u32 length) {
    auto space    = getSpace();
    auto overflow = false;
    if (length > space) {
        if (length > _capacity) {
            length = _capacity;
        }
        overflow = true;
    }
    _write = WrapLogicIndex(_write + length);
    if (overflow) {
        _read = WrapLogicIndex(_write - _capacity);
    }
    return overflow;
}
template <typename TE>
u32 CircularBuffer<TE>::write(const TE *data, u32 length, bool allowCover) {
    auto overflow = false;
    auto space    = getSpace();
    if (length > space) {
        if (allowCover) {
            if (length > _capacity) {
                length = _capacity;
                data += length - _capacity;
            }
            overflow = true;
        } else {
            length = space;
        }
    } else {
    }
    auto writeMemIndex          = WrapMemIndex(_write);
    auto roomFrom_WriteToBotton = _capacity - writeMemIndex;
    if (length <= roomFrom_WriteToBotton) {
        memcpy(_buffer + writeMemIndex, data, length * sizeof(TE));
    } else {
        memcpy(_buffer + writeMemIndex, data, roomFrom_WriteToBotton * sizeof(TE));
        memcpy(_buffer, data + roomFrom_WriteToBotton,
               (length - roomFrom_WriteToBotton) * sizeof(TE));
    }
    _write = WrapLogicIndex(_write + length);

    if (overflow) {
        _read = WrapLogicIndex(_write - _capacity);
    }

    return length;
}
template <typename TE>
CircularBuffer<TE>::CircularBuffer(Slice buffer) : _buffer(buffer.data), _capacity(buffer.size) {
    ASSERT(!((_capacity - 1) & _capacity), "Slice size MUST be power of 2!");
    _write = 0;
    _read  = 0;
}

template <typename TE>
TE *CircularBuffer<TE>::getWritePtr() const {
    return &_buffer[WrapMemIndex(_write)];
};

template <typename TE>
TE *CircularBuffer<TE>::getReadPtr() const {
    return &_buffer[WrapMemIndex(_read)];
};
template <typename TE>
u32 CircularBuffer<TE>::getSizeWithoutMemWrap() const {
    return std::min((_write - _read) & ((_capacity << 1) - 1), _capacity - WrapMemIndex(_read));
};
template <typename TE>
TE *CircularBuffer<TE>::peekPtr(u32 offset, bool force) {
    if (force) {
        return &_buffer[WrapMemIndex(_read)];
    } else {
        if (offset >= getSize()) {
            return nullptr;
        }
        return &_buffer[WrapMemIndex(_read + offset)];
    }
}

template <typename TE>
u32 CircularBuffer<TE>::getLengthByMemIndex(u32 end, u32 start) {
    return (end - start) & (_capacity - 1);
};
template <typename TE>
u32 CircularBuffer<TE>::getMemCapacity() const {
    return _capacity * sizeof(TE);
};
template <typename TE>
u32 CircularBuffer<TE>::getDataWidth() const {
    return sizeof(TE);
};
template <typename TE>
bool CircularBuffer<TE>::isFull() const {
    return _read == (_write ^ _capacity);
};
template <typename TE>
bool CircularBuffer<TE>::isEmpty() const {
    return _write == _read;
};
template <typename TE>
u32 CircularBuffer<TE>::getSize() const {
    return (_write - _read) & ((_capacity << 1) - 1);
};

template <typename TE>
u32 CircularBuffer<TE>::getCapacity() const {
    return _capacity;
};

template <typename TE>
u32 CircularBuffer<TE>::getSpace() const {
    return _capacity - getSize();
};

template <typename TE>
Result CircularBuffer<TE>::clear() {
    _write = 0;
    _read  = 0;
    return Result();
};

template <typename TE>
TE *CircularBuffer<TE>::getDataPtr() {
    return _buffer;
};

template class CircularBuffer<u8>;

}  // namespace wibot
