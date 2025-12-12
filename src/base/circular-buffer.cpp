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
    const u32 readMemIndex         = WrapMemIndex(_read + start);
    const u32 roomFromReadToBottom = _capacity - readMemIndex;
    const u32 firstPart  = (length < roomFromReadToBottom) ? length : roomFromReadToBottom;
    const u32 secondPart = length - firstPart;
    if (firstPart) {
        memcpy(data, _buffer + readMemIndex, firstPart * sizeof(TE));
    }
    if (secondPart) {
        memcpy(data + firstPart, _buffer, secondPart * sizeof(TE));
    }
    return length;
}

template <typename TE>
u32 CircularBuffer<TE>::read(TE *data, u32 length) {
    auto size = getSize();
    if (length > size) {
        length = size;
    }
    const u32 readMemIndex         = WrapMemIndex(_read);
    const u32 roomFromReadToBottom = _capacity - readMemIndex;
    const u32 firstPart  = (length < roomFromReadToBottom) ? length : roomFromReadToBottom;
    const u32 secondPart = length - firstPart;
    if (firstPart) {
        memcpy(data, _buffer + readMemIndex, firstPart * sizeof(TE));
    }
    if (secondPart) {
        memcpy(data + firstPart, _buffer, secondPart * sizeof(TE));
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
    const u32 writeMemIndex         = WrapMemIndex(_write);
    const u32 roomFromWriteToBottom = _capacity - writeMemIndex;
    const u32 firstPart  = (length < roomFromWriteToBottom) ? length : roomFromWriteToBottom;
    const u32 secondPart = length - firstPart;
    if (firstPart) {
        memcpy(_buffer + writeMemIndex, data, firstPart * sizeof(TE));
    }
    if (secondPart) {
        memcpy(_buffer, data + firstPart, secondPart * sizeof(TE));
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
    const u32 memMask = _capacity - 1;
    return &_buffer[_write & memMask];
};

template <typename TE>
TE *CircularBuffer<TE>::getReadPtr() const {
    const u32 memMask = _capacity - 1;
    return &_buffer[_read & memMask];
};
template <typename TE>
u32 CircularBuffer<TE>::getSizeWithoutMemWrap() const {
    const u32 logicMask = (_capacity << 1) - 1;
    const u32 memMask   = _capacity - 1;
    return std::min(((_write - _read) & logicMask), _capacity - (_read & memMask));
};
template <typename TE>
TE *CircularBuffer<TE>::peekPtr(u32 offset, bool force) {
    if (force) {
        const u32 memMask = _capacity - 1;
        return &_buffer[_read & memMask];
    } else {
        if (offset >= getSize()) {
            return nullptr;
        }
        const u32 memMask = _capacity - 1;
        return &_buffer[(_read + offset) & memMask];
    }
}

template <typename TE>
u32 CircularBuffer<TE>::getLengthByMemIndex(u32 end, u32 start) {
    const u32 memMask = _capacity - 1;
    return (end - start) & memMask;
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
    const u32 logicMask = (_capacity << 1) - 1;
    return (_write - _read) & logicMask;
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
