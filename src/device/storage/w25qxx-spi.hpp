#pragma once

#include "type.hpp"
#include "block.hpp"
#include "peripheral.hpp"
#include "bus.hpp"

namespace wibot {

constexpr u32 kW25qxxPageSize  = 256;
constexpr u32 kW25qxxBlockSize = 4096;
enum W25qxxCommandMode {
    kSpi,
    // Qspi,
    kQpi,
};

enum W25qxxEraseMode {
    k4K,
    k32K,
    k64K,
    kChip,
};

enum W25qxxCmdLineMode : u8 {
    k111 = 0x15,
    k110 = 0x14,
    k101 = 0x11,
    k100 = 0x10,
    k444 = 0x2A,
    k440 = 0x28,
    k404 = 0x22,
    k400 = 0x20,
};

union W25qxxStatus1Register {
    u8 value;
    struct {
        u8 BUSY : 1;
        u8 WEL  : 1;
        u8 BP   : 3;
        u8 TB   : 1;
        u8 SEC  : 1;
        u8 SRP  : 1;
    };
};
union W25qxxStatus2Register {
    u8 value;
    struct {
        u8 SRL : 1;
        u8 QE  : 1;
        u8     : 1;
        u8 LB  : 3;
        u8 CMP : 1;
        u8 SUS : 1;
    };
};
union W25qxxStatus3Register {
    u8 value;
    struct {
        u8          : 2;
        u8 WPS      : 1;
        u8          : 2;
        u8 DRV      : 2;
        u8 HOLD_RST : 1;
    };
};

class W25qxxSpi {
   public:
    W25qxxSpi(SpiMaster *spi, u32 timeout);

    Result reset();

    Result switchMode(W25qxxCommandMode cmdMode);

    Result eraseBlock(u32 address);
    Result eraseChip();
    Result readId(u32 &mdId, u32 &jedecId);

    Result readMedia(void *data, u32 num, u32 size);
    Result writeMedia(void *data, u32 num, u32 size);
    Result eraseMedia(u32 num, u32 size);

   private:
    Result _getStatus(u8 reg_num, u8 &status);
    Result _setStatus(u8 reg_num, u8 status);

    Result _busyWaiting();
    Result _enableWriteCommand();
    Result _writeCommand(u8 *pData, u32 writeAddr, u32 dataSize);
    Result _readCommmand(u8 *pData, u32 readAddr, u32 size);

    Result _eraseCommand(W25qxxEraseMode mode, u32 address);
    Result _eraseChipCommand();
    Result _resetCommand();
    Result _spiWriteRead(u8 *writeData, u16 writeLength, u8 *readData, u16 readLength);
    Result _spiWriteWrite(u8 *writeData, u16 writeLength, u8 *data, u16 length);

   private:
    SpiMaster *_spi;
    u32        _timeout;
};

class W25qxxSpiBlock : public Block {
   public:
    W25qxxSpiBlock(W25qxxSpi &w25qxx, Slice buffer);

   protected:
    W25qxxSpi &_w25qxx;
    Result     readMedia(void *data, u32 num, u32 size) override;
    Result     writeMedia(void *data, u32 num, u32 size) override;
    Result     eraseMedia(u32 num, u32 size) override;
};

}  // namespace wibot
