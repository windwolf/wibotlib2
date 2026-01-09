#include "w25qxx-spi.hpp"

/* Write Operations */
#define W25QXX_SPI_WRITE_ENABLE_CMD             0x06
#define W25QXX_SPI_VOLATILE_SR_WRITE_ENABLE_CMD 0x50
#define W25QXX_SPI_WRITE_DISABLE_CMD            0x04

/* Identification Operations */
#define W25QXX_SPI_RELEASE_POWER_DOWN_CMD 0xAB
#define W25QXX_SPI_READ_ID_CMD            0x90
#define W25QXX_SPI_READ_JEDEC_ID_CMD      0x9F
#define W25QXX_SPI_READ_UNIQUE_ID_CMD     0x4B

/* Read Operations */
#define W25QXX_SPI_READ_CMD      0x03
#define W25QXX_SPI_FAST_READ_CMD 0x0B

/* Program Operations */
#define W25QXX_SPI_PAGE_PROG_CMD 0x02

/* Erase Operations */
#define W25QXX_SPI_SECTOR_ERASE_4K_CMD 0x20
#define W25QXX_SPI_BLOCK_ERASE_32K_CMD 0x52
#define W25QXX_SPI_BLOCK_ERASE_64K_CMD 0xD8
#define W25QXX_SPI_CHIP_ERASE_CMD      0xC7

/* Regiser Operations */
#define W25QXX_SPI_READ_STATUS_REG1_CMD 0x05
#define W25QXX_SPI_READ_STATUS_REG2_CMD 0x35
#define W25QXX_SPI_READ_STATUS_REG3_CMD 0x15

#define W25QXX_SPI_WRITE_STATUS_REG1_CMD 0x01
#define W25QXX_SPI_WRITE_STATUS_REG2_CMD 0x31
#define W25QXX_SPI_WRITE_STATUS_REG3_CMD 0x11

/* Security Register Operations */
#define W25QXX_SPI_READ_SFDP_REG_CMD        0x5A
#define W25QXX_SPI_ERASE_SECURITY_REG_CMD   0x44
#define W25QXX_SPI_PROGRAM_SECURITY_REG_CMD 0x42
#define W25QXX_SPI_READ_SECURITY_REG_CMD    0x48

/* Lock Operations */
#define W25QXX_SPI_GLOBAL_BLOCK_LOCK_CMD       0x7E
#define W25QXX_SPI_GLOBAL_BLOCK_UNLOCK_CMD     0x98
#define W25QXX_SPI_READ_BLOCK_LOCK_CMD         0x3D
#define W25QXX_SPI_INDEVIDUAL_BLOCK_LOCK_CMD   0x36
#define W25QXX_SPI_INDEVIDUAL_BLOCK_UNLOCK_CMD 0x39

#define W25QXX_SPI_PROG_ERASE_SUSPEND_CMD 0x75
#define W25QXX_SPI_PROG_ERASE_RESUME_CMD  0x7A
#define W25QXX_SPI_PONER_DOWN_CMD         0xB9

#define W25QXX_SPI_ENTER_QPI_MODE_CMD 0x38
#define W25QXX_SPI_ENABLE_RESET_CMD   0x66
#define W25QXX_SPI_RESET_DEVICE_CMD   0x99

#define W25QXX_QSPI_INPUT_PAGE_PROG_CMD     0x32  // 1-1-4
#define W25QXX_QSPI_FAST_READ_OUTPUT_CMD    0x6B  // 1-1-4
#define W25QXX_QSPI_MFTR_DEVICE_ID_IO_CMD   0x94  // 1-4-4
#define W25QXX_QSPI_FAST_READ_IO_CMD        0xEB  // 1-4-4
#define W25QXX_QSPI_SET_BURST_WITH_WRAP_CMD 0x77  // 1-4-4

/* Write Operations */
#define W25QXX_QPI_WRITE_ENABLE_CMD             0x06
#define W25QXX_QPI_VOLATILE_SR_WRITE_ENABLE_CMD 0x50
#define W25QXX_QPI_WRITE_DISABLE_CMD            0x04

/* Identification Operations */
#define W25QXX_QPI_RELEASE_POWER_DOWN_CMD  0xAB
#define W25QXX_QPI_READ_ID_CMD             0x90
#define W25QXX_QPI_READ_JEDEC_ID_CMD       0x9F
#define W25QXX_QPI_SET_READ_PARAMETERS_CMD 0xC0

/* Read Operations */
#define W25QXX_QPI_FAST_READ_CMD            0x0B
#define W25QXX_QPI_BURST_READ_WITH_WRAP_CMD 0x0C
#define W25QXX_QPI_FAST_READ_QUAD_IO_CMD    0xEB

/* Program Operations */
#define W25QXX_QPI_PAGE_PROG_CMD 0x02

/* Erase Operations */
#define W25QXX_QPI_SECTOR_ERASE_4K_CMD 0x20
#define W25QXX_QPI_BLOCK_ERASE_32K_CMD 0x52
#define W25QXX_QPI_BLOCK_ERASE_64K_CMD 0xD8
#define W25QXX_QPI_CHIP_ERASE_CMD      0xC7

/* Regiser Operations */
#define W25QXX_QPI_READ_STATUS_REG1_CMD 0x05
#define W25QXX_QPI_READ_STATUS_REG2_CMD 0x35
#define W25QXX_QPI_READ_STATUS_REG3_CMD 0x15

#define W25QXX_QPI_WRITE_STATUS_REG1_CMD 0x01
#define W25QXX_QPI_WRITE_STATUS_REG2_CMD 0x31
#define W25QXX_QPI_WRITE_STATUS_REG3_CMD 0x11

/* Lock Operations */
#define W25QXX_QPI_GLOBAL_BLOCK_LOCK_CMD       0x7E
#define W25QXX_QPI_GLOBAL_BLOCK_UNLOCK_CMD     0x7E
#define W25QXX_QPI_READ_BLOCK_LOCK_CMD         0x3D
#define W25QXX_QPI_INDEVIDUAL_BLOCK_LOCK_CMD   0x36
#define W25QXX_QPI_INDEVIDUAL_BLOCK_UNLOCK_CMD 0x39

#define W25QXX_QPI_PROG_ERASE_SUSPEND_CMD 0x75
#define W25QXX_QPI_PROG_ERASE_RESUME_CMD  0x7A
#define W25QXX_QPI_PONER_DOWN_CMD         0xB9

#define W25QXX_QPI_ENTER_QPI_MODE_CMD 0x38
#define W25QXX_QPI_ENABLE_RESET_CMD   0x66
#define W25QXX_QPI_RESET_DEVICE_CMD   0x99
#define W25QXX_QPI_EXIT_QPI_MODE_CMD  0xFF

#define W25QXX_EVENT_OP_BUSY 0x01
#define W25QXX_EVENT_OP_CPLT 0x02

// #define W25QXX_EVENTS_OP_BUSY_SET(instance)
// EVENTS_SET_FLAGS(instance->events, W25QXX_EVENT_OP_BUSY) #define
// W25QXX_EVENTS_OP_BUSY_RESET(instance) EVENTS_RESET_FLAGS(instance->events,
// W25QXX_EVENT_OP_BUSY) #define W25QXX_EVENTS_DEVICE_BUSY_SET(instance)
// EVENTS_SET_FLAGS(instance->events, W25QXX_EVENT_DEVICE_BUSY) #define
// W25QXX_EVENTS_DEVICE_BUSY_RESET(instance)
// EVENTS_RESET_FLAGS(instance->events, W25QXX_EVENT_DEVICE_BUSY)

namespace wibot::device {

W25qxxSpi::W25qxxSpi(SpiMaster &spi, u32 timeout) : _spi(spi), _timeout(timeout) {};

Result W25qxxSpi::_spiWriteRead(u8 *writeData, u16 writeLength, u8 *readData, u16 readLength) {
    Result rst;
    do {
        rst = _spi.begin();
        if (rst != Result::kOk) {
            break;
        }
        if (writeLength > 0) {
            auto ar = _spi.write(Slice(writeData, writeLength));
            rst     = ar.wait(_timeout);
            if (rst != Result::kOk) {
                break;
            }
        }
        if (readLength > 0) {
            auto ar = _spi.read(Slice(readData, readLength));
            rst     = ar.wait(_timeout);
            if (rst != Result::kOk) {
                break;
            }
        }
    } while (0);

    _spi.end();
    return rst;
};
Result W25qxxSpi::_spiWriteWrite(u8 *writeData, u16 writeLength, u8 *data, u16 length) {
    Result rst;
    do {
        rst = _spi.begin();
        if (rst != Result::kOk) {
            break;
        }
        if (writeLength > 0) {
            auto ar = _spi.write(Slice(writeData, writeLength));
            rst     = ar.wait(_timeout);
            if (rst != Result::kOk) {
                break;
            }
        }
        if (length > 0) {
            auto ar = _spi.write(Slice(data, length));
            rst     = ar.wait(_timeout);
            if (rst != Result::kOk) {
                break;
            }
        }
    } while (0);

    _spi.end();
    return rst;
};
Result W25qxxSpi::reset() {
    Result rst;

    rst = _resetCommand();
    if (rst != Result::kOk) {
        return rst;
    }
    W25qxxStatus2Register reg2;
    rst = _getStatus(2, reg2.value);
    if (rst != Result::kOk) {
        return rst;
    }
    reg2.QE = 0;
    rst     = _setStatus(2, reg2.value);
    if (rst != Result::kOk) {
        return rst;
    }
    rst = _busyWaiting();
    if (rst != Result::kOk) {
        return rst;
    }
    return rst;
};

Result W25qxxSpi::eraseBlock(u32 address) {
    Result rst;
    rst = _enableWriteCommand();
    if (rst != Result::kOk) {
        return rst;
    }

    rst = _eraseCommand(k4K, address);
    if (rst != Result::kOk) {
        return rst;
    }
    return _busyWaiting();
};
Result W25qxxSpi::eraseChip() {
    Result rst;
    rst = _enableWriteCommand();
    if (rst != Result::kOk) {
        return rst;
    }
    rst = _eraseChipCommand();
    if (rst != Result::kOk) {
        return rst;
    }
    return _busyWaiting();
};

Result W25qxxSpi::readMedia(void *data, u32 num, u32 size) {
    return _readCommmand((u8 *)data, num, size);
};
Result W25qxxSpi::writeMedia(void *data, u32 num, u32 size) {
    Result rst;
    do {
        rst = _enableWriteCommand();
        if (rst != Result::kOk) {
            break;
        }

        rst = _writeCommand((u8 *)data, num, size);
        if (rst != Result::kOk) {
            break;
        }
        rst = _busyWaiting();
    } while (0);
    return rst;
};

Result W25qxxSpi::eraseMedia(u32 num, u32 size) {
    Result rst;
    do {
        u32 blkBeginAddr = num & ~(kW25qxxBlockSize - 1);
        u32 blkEndAddr   = (num + size - 1) & ~(kW25qxxBlockSize - 1);
        u32 curAddr      = blkBeginAddr;
        do {
            rst = eraseBlock(curAddr);
            if (rst != Result::kOk) {
                break;
            }
            curAddr += kW25qxxBlockSize;

        } while (curAddr <= blkEndAddr);
    } while (0);
    return rst;
};

Result W25qxxSpi::_getStatus(u8 reg_num, u8 &status) {
    Result rst;

    u8 cmd = (reg_num == 1) ? W25QXX_SPI_READ_STATUS_REG1_CMD
                            : ((reg_num == 2) ? W25QXX_SPI_READ_STATUS_REG2_CMD
                                              : W25QXX_SPI_READ_STATUS_REG3_CMD);
    rst    = _spiWriteRead(&cmd, 1, &status, 1);
    return rst;
};
Result W25qxxSpi::_setStatus(u8 reg_num, u8 status) {
    Result rst;
    u8     buf[2];
    buf[0] = (reg_num == 1) ? W25QXX_SPI_WRITE_STATUS_REG1_CMD
                            : ((reg_num == 2) ? W25QXX_SPI_WRITE_STATUS_REG2_CMD
                                              : W25QXX_SPI_WRITE_STATUS_REG3_CMD);
    buf[1] = status;

    rst = _spiWriteRead(buf, 2, nullptr, 0);
    return rst;
};

Result W25qxxSpi::readId(u32 &mdId, u32 &jedecId) {
    Result rst;
    u8     cmd = W25QXX_SPI_READ_JEDEC_ID_CMD;

    u8 data[3];
    rst = _spiWriteRead(&cmd, 1, data, 3);
    if (rst != Result::kOk) {
        return rst;
    }
    mdId    = data[0];
    jedecId = (data[1] << 8) | data[2];
    return rst;
};
Result W25qxxSpi::_busyWaiting() {
    Result                rst;
    W25qxxStatus1Register status1;
    do {
        rst = _getStatus(1, status1.value);

        if (rst != Result::kOk) {
            return rst;
        }
        if (!status1.BUSY) {
            return Result::kOk;
        } else {
            os::sleep(1);
        }
    } while (1);
};

Result W25qxxSpi::_enableWriteCommand() {
    Result rst;
    u8     cmd = W25QXX_SPI_WRITE_ENABLE_CMD;

    rst = _spiWriteRead(&cmd, 1, nullptr, 0);
    return rst;
};

Result W25qxxSpi::_writeCommand(u8 *pData, u32 writeAddr, u32 dataSize) {
    Result rst;
    u8     buf[4];
    buf[0] = W25QXX_SPI_PAGE_PROG_CMD;
    buf[1] = (writeAddr >> 16) & 0xff;
    buf[2] = (writeAddr >> 8) & 0xff;
    buf[3] = (writeAddr) & 0xff;
    rst    = _spiWriteWrite(buf, 4, pData, dataSize);
    return rst;
};
Result W25qxxSpi::_readCommmand(u8 *pData, u32 readAddr, u32 size) {
    Result rst;
    u8     buf[4];
    buf[0] = W25QXX_SPI_READ_CMD;
    buf[1] = (readAddr >> 16) & 0xff;
    buf[2] = (readAddr >> 8) & 0xff;
    buf[3] = (readAddr) & 0xff;

    rst = _spiWriteRead(buf, 4, pData, size);
    return rst;
};

Result W25qxxSpi::_resetCommand() {
    Result rst = Result::kOk;

    return rst;
};

Result W25qxxSpi::_eraseCommand(W25qxxEraseMode mode, u32 address) {
    Result rst;
    u8     buf[4];
    buf[0] = (mode == k4K) ? W25QXX_SPI_SECTOR_ERASE_4K_CMD
                           : ((mode == k32K) ? W25QXX_SPI_BLOCK_ERASE_32K_CMD
                                             : W25QXX_SPI_BLOCK_ERASE_64K_CMD);
    buf[1] = (address >> 16) & 0xff;
    buf[2] = (address >> 0) & 0xff;
    buf[3] = (address) & 0xff;

    rst = _spiWriteRead(buf, 4, nullptr, 0);
    return rst;
};
Result W25qxxSpi::_eraseChipCommand() {
    Result rst;
    u8     cmd = W25QXX_SPI_CHIP_ERASE_CMD;

    rst = _spiWriteRead(&cmd, 1, nullptr, 0);
    return rst;
};

W25qxxSpiBlock::W25qxxSpiBlock(W25qxxSpi &w25qxx, Slice buffer) : hal::Block(buffer), _w25qxx(w25qxx) {
    setConfig(hal::Block::Config{
        .readBlockSize        = 0,
        .writeBlockSize       = kW25qxxPageSize,
        .eraseBlockSize       = kW25qxxBlockSize,
        .readMode             = hal::Block::Mode::kRandom,
        .writeMode            = hal::Block::Mode::kWrap,
        .eraseMode            = hal::Block::Mode::kRandomBlock,
        .needEraseBeforeWrite = true,
    });
};

Result W25qxxSpiBlock::readMedia(void *data, u32 num, u32 size) {
    return _w25qxx.readMedia(data, num, size);
};

Result W25qxxSpiBlock::writeMedia(void *data, u32 num, u32 size) {
    return _w25qxx.writeMedia(data, num, size);
};

Result W25qxxSpiBlock::eraseMedia(u32 num, u32 size) {
    return _w25qxx.eraseMedia(num, size);
}

}  // namespace wibot
