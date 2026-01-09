#pragma once

#include "type.hpp"
#include "buffer.hpp"

namespace wibot::hal {

class Block {
   public:
    enum class Mode : u8 {
        kRandom      = 0x00,
        kWrap        = 0x01,
        kBlockwise   = 0x02,
        kRandomBlock = 0x03,
        kBlock       = 0x04,
    };

    struct Config {
        u32  readBlockSize;
        u32  writeBlockSize;
        u32  eraseBlockSize;
        Mode readMode;
        Mode writeMode;
        Mode eraseMode;
        bool needEraseBeforeWrite;
    };

   public:
    Block(Slice &buffer);
    Result setConfig(const Config &config);

    Result read(void *data, u32 address, u32 size);
    Result write(void *data, u32 address, u32 size);
    Result erase(u32 address, u32 size);

   protected:
    virtual Result readMedia(void *data, u32 num, u32 size)  = 0;
    virtual Result writeMedia(void *data, u32 num, u32 size) = 0;
    virtual Result eraseMedia(u32 num, u32 size)             = 0;

   private:
    struct {
        u32 maxBlockSize;
        u32 readBlockSizeBits;
        u32 writeBlockSizeBits;
        u32 eraseBlockSizeBits;
        u32 readBlockSizeMask;
        u32 writeBlockSizeMask;
        u32 eraseBlockSizeMask;
    } _calculatedConfig;

    Config _config;
    Slice  _buffer;
    Result _writeDirectly(void *data, u32 address, u32 size);
    Result _calculateConfig();
};
}  // namespace wibot::hal
