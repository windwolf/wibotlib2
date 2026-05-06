#include "block.hpp"

#include "math/operator.hpp"

#include "logger.hpp"

#define min(a, b) (((a) <= (b)) ? (a) : (b))
#define max(a, b) (((a) >= (b)) ? (a) : (b))

LOGGER("block")
namespace wibot {

Block::Block(Slice &buffer) : _buffer(buffer) {};

Result Block::setConfig(const Config &config) {
    _config = config;
    return _calculateConfig();
};
Result Block::_calculateConfig() {
    ASSERT(_config.readMode != Mode::kRandomBlock, "readReg should not RANDOM_BLOCK");
    ASSERT(_config.writeMode != Mode::kRandomBlock, "writeReg should not RANDOM_BLOCK");
    if (_config.readBlockSize == 0 || _config.writeBlockSize == 0 ||
        _config.eraseBlockSize == 0) {
        return Result::kInvalidParameter;
    }
    if ((_config.readBlockSize & (_config.readBlockSize - 1)) != 0 ||
        (_config.writeBlockSize & (_config.writeBlockSize - 1)) != 0 ||
        (_config.eraseBlockSize & (_config.eraseBlockSize - 1)) != 0) {
        return Result::kInvalidParameter;
    }
    ASSERT(_config.writeBlockSize <= _config.eraseBlockSize,
           "writeReg block should not be great then erase block size.");
    ASSERT(_buffer.size >= max(_config.readBlockSize, _config.eraseBlockSize),
           "Not enough buffer size.");

    u32 maxBlockSize = (_config.readBlockSize > _config.writeBlockSize) ? _config.readBlockSize
                                                                        : _config.writeBlockSize;
    maxBlockSize = (maxBlockSize > _config.eraseBlockSize) ? maxBlockSize : _config.eraseBlockSize;
    _calculatedConfig.maxBlockSize = maxBlockSize;
    Math math;
    _calculatedConfig.readBlockSizeBits  = math.log2(_config.readBlockSize);
    _calculatedConfig.writeBlockSizeBits = math.log2(_config.writeBlockSize);
    _calculatedConfig.eraseBlockSizeBits = math.log2(_config.eraseBlockSize);
    _calculatedConfig.readBlockSizeMask  = _config.readBlockSize - 1;
    _calculatedConfig.writeBlockSizeMask = _config.writeBlockSize - 1;
    _calculatedConfig.eraseBlockSizeMask = _config.eraseBlockSize - 1;
    return Result::kOk;
};

Result Block::read(void *data, u32 address, u32 size) {
    Result rst;

    do {
        if (_config.readMode == Mode::kRandom) {
            rst = readMedia(data, address, size);
            if (rst != Result::kOk) {
                break;
            }
        } else if (_config.readMode == Mode::kBlockwise) {
            u32 blkAddress = address & ~(_calculatedConfig.readBlockSizeMask);
            u32 blkSize    = size & ~(_calculatedConfig.readBlockSizeMask);
            if ((address != blkAddress) || (size != blkSize)) {
                // not aligned
                return Result::kInvalidParameter;
            }
            rst = readMedia(data, address >> _calculatedConfig.readBlockSizeBits,
                            size >> _calculatedConfig.readBlockSizeBits);
            if (rst != Result::kOk) {
                break;
            }
        } else if (_config.readMode == Mode::kBlock) {
            rst = readMedia(data, address, size);
            if (rst != Result::kOk) {
                break;
            }
        } else if (_config.readMode == Mode::kWrap) {
            u32 sizeInBlock, remainSize;
            remainSize = size;

            u8 *curDataPtr = (u8 *)data;
            do {
                sizeInBlock =
                    _config.readBlockSize - (address & (_calculatedConfig.readBlockSizeMask));
                if (sizeInBlock > remainSize) {
                    sizeInBlock = remainSize;
                }

                remainSize -= sizeInBlock;

                rst = readMedia(curDataPtr, address, sizeInBlock);

                if (rst != Result::kOk) {
                    break;
                }

                curDataPtr += sizeInBlock;
                address += sizeInBlock;
            } while (remainSize > 0);

            rst = Result::kOk;
            break;
        } else {
            rst = Result::kNotSupport;
            break;
        }
    } while (0);
    return rst;
};
Result Block::write(void *data, u32 address, u32 size) {
    Result rst;

    do {
        if (_config.writeMode == Mode::kBlockwise) {
            u32 blkAddress = address & ~(_calculatedConfig.writeBlockSizeMask);
            u32 blkSize    = size & ~(_calculatedConfig.writeBlockSizeMask);
            if ((address != blkAddress) || (size != blkSize)) {
                // not aligned
                rst = Result::kInvalidParameter;
                break;
            }
        }
        if (_config.needEraseBeforeWrite) {
            u8 *buffer = (u8 *)this->_buffer.data;

            u32 wRemainSize = size;
            u32 wAddr       = address;

            u8 *wData     = (u8 *)data;
            u32 erBlkSize = max(_config.eraseBlockSize, _config.readBlockSize);
            u32 erBlkMask = erBlkSize - 1;
            do {
                u32 erBlkAddr  = wAddr & ~erBlkMask;
                u32 wPosInBlk  = wAddr & erBlkMask;
                u32 wSizeInBlk = min(wRemainSize, erBlkSize - wPosInBlk);

                if ((erBlkAddr != wAddr) || (wRemainSize < erBlkSize)) {
                    // address not aligned to erBlock or tail fragment.
                    // read->memcpy->erase->write.
                    rst = read(buffer, erBlkAddr, erBlkSize);  // read entire block
                    if (rst != Result::kOk) {
                        break;
                    }
                    memcpy((void *)(buffer + wPosInBlk), (const void *)wData, wSizeInBlk);
                    rst = erase(erBlkAddr, erBlkSize);
                    if (rst != Result::kOk) {
                        break;
                    }
                    rst = _writeDirectly(buffer, erBlkAddr, erBlkSize);  // write entire block
                    if (rst != Result::kOk) {
                        break;
                    }
                } else {
                    // address aligned to erBlock. middle entire blocks. directly
                    // erase->write.
                    u32 blkCount      = wRemainSize / erBlkSize;
                    u32 writeSize     = erBlkSize * blkCount;
                    rst               = erase(erBlkAddr, writeSize);
                    if (rst != Result::kOk) {
                        break;
                    }
                    rst = _writeDirectly(wData, erBlkAddr, writeSize);
                    if (rst != Result::kOk) {
                        break;
                    }
                    wSizeInBlk = writeSize;
                }

                wAddr += wSizeInBlk;
                wData += wSizeInBlk;
                wRemainSize -= wSizeInBlk;
            } while (wRemainSize > 0);
            return Result::kOk;
        } else {
            rst = _writeDirectly(data, address, size);
            if (rst != Result::kOk) {
                break;
            }
        }
    } while (0);

    return rst;
};
Result Block::erase(u32 address, u32 size) {
    Result rst;

    do {
        // TODO: simplfy these mode.
        if (_config.eraseMode == Mode::kRandomBlock) {
            rst = eraseMedia(address, size);
            if (rst != Result::kOk) {
                break;
            }
        } else if (_config.eraseMode == Mode::kRandom) {
            rst = eraseMedia(address, size);
            if (rst != Result::kOk) {
                break;
            }
        } else if (_config.eraseMode == Mode::kBlock) {
            rst = eraseMedia(address, size);
            if (rst != Result::kOk) {
                break;
            }
        } else if (_config.eraseMode == Mode::kBlockwise) {
            u32 blkAddress = address & ~(_calculatedConfig.eraseBlockSizeMask);
            u32 blkSize    = size & ~(_calculatedConfig.eraseBlockSizeMask);
            if ((address != blkAddress) || (size != blkSize)) {
                // not aligned
                rst = Result::kInvalidParameter;
                break;
            }
            rst = eraseMedia(address >> (_calculatedConfig.eraseBlockSizeBits),
                             size >> (_calculatedConfig.eraseBlockSizeBits));
            if (rst != Result::kOk) {
                break;
            }
        } else if (_config.eraseMode == Mode::kWrap) {
            rst = Result::kOk;
        } else {
            rst = Result::kNotSupport;
        }
    } while (0);

    return rst;
};

Result Block::_writeDirectly(void *data, u32 address, u32 size) {
    Result rst;
    if (_config.writeMode == Mode::kRandom) {
        rst = writeMedia(data, address, size);
        if (rst != Result::kOk) {
            return rst;
        }
    } else if (_config.writeMode == Mode::kBlockwise) {
        rst = writeMedia(data, address >> (_calculatedConfig.writeBlockSizeBits),
                         size >> (_calculatedConfig.writeBlockSizeBits));
        if (rst != Result::kOk) {
            return rst;
        }
    } else if (_config.writeMode == Mode::kBlock) {
        rst = writeMedia(data, address, size);
        if (rst != Result::kOk) {
            return rst;
        }
    } else if (_config.writeMode == Mode::kWrap) {
        u32 sizeInBlock, remainSize;
        remainSize = size;

        u8 *curDataPtr = (u8 *)data;
        do {
            sizeInBlock = _config.writeBlockSize - (address & _calculatedConfig.writeBlockSizeMask);
            if (sizeInBlock > remainSize) {
                sizeInBlock = remainSize;
            }
            rst = writeMedia(curDataPtr, address, sizeInBlock);
            if (rst != Result::kOk) {
                return rst;
            }

            remainSize -= sizeInBlock;
            curDataPtr += sizeInBlock;
            address += sizeInBlock;
        } while (remainSize > 0);

        return Result::kOk;
    }

    return Result::kNotSupport;
};

}  // namespace wibot
