#pragma once

#include "../pipeline.hpp"

namespace wibot {

/**
 * @brief 内存数组源 INode 实现
 * 
 * 循环读取预设数组中的值
 */
template <typename T, u16 MaxSize>
class MemorySourceNode : public INode {
   public:
    struct Storage {
        T   buffer[MaxSize]{};
        u16 size{0};
        u16 index{0};
    };

    struct Outputs {
        Out<T> x;
    } outputs;

    /**
     * @brief 构造函数
     */
    explicit MemorySourceNode(Storage& storage) : _storage(storage) {
    }

    bool ready() override {
        return outputs.x.bound() && _storage.size > 0;
    }

    void process() override {
        if (_storage.size > 0) {
            outputs.x.ref() = _storage.buffer[_storage.index];
            _storage.index  = (_storage.index + 1) % _storage.size;
        }
    }

    void reset() override {
        _storage.index = 0;
    }

    /**
     * @brief 设置数组内容
     */
    void setBuffer(const T* data, u16 length) {
        u16 copyLen = (length < MaxSize) ? length : MaxSize;
        for (u16 i = 0; i < copyLen; ++i) {
            _storage.buffer[i] = data[i];
        }
        _storage.size  = copyLen;
        _storage.index = 0;
    }

   private:
    Storage& _storage;
};

} // namespace wibot

