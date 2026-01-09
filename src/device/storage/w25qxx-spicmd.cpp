//#include "w25qxx-spicmd.hpp"
//
///* Write Operations */
//#define W25QXX_SPI_WRITE_ENABLE_CMD             0x06
//#define W25QXX_SPI_VOLATILE_SR_WRITE_ENABLE_CMD 0x50
//#define W25QXX_SPI_WRITE_DISABLE_CMD            0x04
//
///* Identification Operations */
//#define W25QXX_SPI_RELEASE_POWER_DOWN_CMD 0xAB
//#define W25QXX_SPI_READ_ID_CMD            0x90
//#define W25QXX_SPI_READ_JEDEC_ID_CMD      0x9F
//#define W25QXX_SPI_READ_UNIQUE_ID_CMD     0x4B
//
///* Read Operations */
//#define W25QXX_SPI_READ_CMD      0x03
//#define W25QXX_SPI_FAST_READ_CMD 0x0B
//
///* Program Operations */
//#define W25QXX_SPI_PAGE_PROG_CMD 0x02
//
///* Erase Operations */
//#define W25QXX_SPI_SECTOR_ERASE_4K_CMD 0x20
//#define W25QXX_SPI_BLOCK_ERASE_32K_CMD 0x52
//#define W25QXX_SPI_BLOCK_ERASE_64K_CMD 0xD8
//#define W25QXX_SPI_CHIP_ERASE_CMD      0xC7
//
///* Regiser Operations */
//#define W25QXX_SPI_READ_STATUS_REG1_CMD 0x05
//#define W25QXX_SPI_READ_STATUS_REG2_CMD 0x35
//#define W25QXX_SPI_READ_STATUS_REG3_CMD 0x15
//
//#define W25QXX_SPI_WRITE_STATUS_REG1_CMD 0x01
//#define W25QXX_SPI_WRITE_STATUS_REG2_CMD 0x31
//#define W25QXX_SPI_WRITE_STATUS_REG3_CMD 0x11
//
///* Security Register Operations */
//#define W25QXX_SPI_READ_SFDP_REG_CMD        0x5A
//#define W25QXX_SPI_ERASE_SECURITY_REG_CMD   0x44
//#define W25QXX_SPI_PROGRAM_SECURITY_REG_CMD 0x42
//#define W25QXX_SPI_READ_SECURITY_REG_CMD    0x48
//
///* Lock Operations */
//#define W25QXX_SPI_GLOBAL_BLOCK_LOCK_CMD       0x7E
//#define W25QXX_SPI_GLOBAL_BLOCK_UNLOCK_CMD     0x98
//#define W25QXX_SPI_READ_BLOCK_LOCK_CMD         0x3D
//#define W25QXX_SPI_INDEVIDUAL_BLOCK_LOCK_CMD   0x36
//#define W25QXX_SPI_INDEVIDUAL_BLOCK_UNLOCK_CMD 0x39
//
//#define W25QXX_SPI_PROG_ERASE_SUSPEND_CMD 0x75
//#define W25QXX_SPI_PROG_ERASE_RESUME_CMD  0x7A
//#define W25QXX_SPI_PONER_DOWN_CMD         0xB9
//
//#define W25QXX_SPI_ENTER_QPI_MODE_CMD 0x38
//#define W25QXX_SPI_ENABLE_RESET_CMD   0x66
//#define W25QXX_SPI_RESET_DEVICE_CMD   0x99
//
//#define W25QXX_QSPI_INPUT_PAGE_PROG_CMD     0x32  // 1-1-4
//#define W25QXX_QSPI_FAST_READ_OUTPUT_CMD    0x6B  // 1-1-4
//#define W25QXX_QSPI_MFTR_DEVICE_ID_IO_CMD   0x94  // 1-4-4
//#define W25QXX_QSPI_FAST_READ_IO_CMD        0xEB  // 1-4-4
//#define W25QXX_QSPI_SET_BURST_WITH_WRAP_CMD 0x77  // 1-4-4
//
///* Write Operations */
//#define W25QXX_QPI_WRITE_ENABLE_CMD             0x06
//#define W25QXX_QPI_VOLATILE_SR_WRITE_ENABLE_CMD 0x50
//#define W25QXX_QPI_WRITE_DISABLE_CMD            0x04
//
///* Identification Operations */
//#define W25QXX_QPI_RELEASE_POWER_DOWN_CMD  0xAB
//#define W25QXX_QPI_READ_ID_CMD             0x90
//#define W25QXX_QPI_READ_JEDEC_ID_CMD       0x9F
//#define W25QXX_QPI_SET_READ_PARAMETERS_CMD 0xC0
//
///* Read Operations */
//#define W25QXX_QPI_FAST_READ_CMD            0x0B
//#define W25QXX_QPI_BURST_READ_WITH_WRAP_CMD 0x0C
//#define W25QXX_QPI_FAST_READ_QUAD_IO_CMD    0xEB
//
///* Program Operations */
//#define W25QXX_QPI_PAGE_PROG_CMD 0x02
//
///* Erase Operations */
//#define W25QXX_QPI_SECTOR_ERASE_4K_CMD 0x20
//#define W25QXX_QPI_BLOCK_ERASE_32K_CMD 0x52
//#define W25QXX_QPI_BLOCK_ERASE_64K_CMD 0xD8
//#define W25QXX_QPI_CHIP_ERASE_CMD      0xC7
//
///* Regiser Operations */
//#define W25QXX_QPI_READ_STATUS_REG1_CMD 0x05
//#define W25QXX_QPI_READ_STATUS_REG2_CMD 0x35
//#define W25QXX_QPI_READ_STATUS_REG3_CMD 0x15
//
//#define W25QXX_QPI_WRITE_STATUS_REG1_CMD 0x01
//#define W25QXX_QPI_WRITE_STATUS_REG2_CMD 0x31
//#define W25QXX_QPI_WRITE_STATUS_REG3_CMD 0x11
//
///* Lock Operations */
//#define W25QXX_QPI_GLOBAL_BLOCK_LOCK_CMD       0x7E
//#define W25QXX_QPI_GLOBAL_BLOCK_UNLOCK_CMD     0x7E
//#define W25QXX_QPI_READ_BLOCK_LOCK_CMD         0x3D
//#define W25QXX_QPI_INDEVIDUAL_BLOCK_LOCK_CMD   0x36
//#define W25QXX_QPI_INDEVIDUAL_BLOCK_UNLOCK_CMD 0x39
//
//#define W25QXX_QPI_PROG_ERASE_SUSPEND_CMD 0x75
//#define W25QXX_QPI_PROG_ERASE_RESUME_CMD  0x7A
//#define W25QXX_QPI_PONER_DOWN_CMD         0xB9
//
//#define W25QXX_QPI_ENTER_QPI_MODE_CMD 0x38
//#define W25QXX_QPI_ENABLE_RESET_CMD   0x66
//#define W25QXX_QPI_RESET_DEVICE_CMD   0x99
//#define W25QXX_QPI_EXIT_QPI_MODE_CMD  0xFF
//
//#define W25QXX_EVENT_OP_BUSY 0x01
//#define W25QXX_EVENT_OP_CPLT 0x02
//
//// #define W25QXX_EVENTS_OP_BUSY_SET(instance)
//// EVENTS_SET_FLAGS(instance->events, W25QXX_EVENT_OP_BUSY) #define
//// W25QXX_EVENTS_OP_BUSY_RESET(instance) EVENTS_RESET_FLAGS(instance->events,
//// W25QXX_EVENT_OP_BUSY) #define W25QXX_EVENTS_DEVICE_BUSY_SET(instance)
//// EVENTS_SET_FLAGS(instance->events, W25QXX_EVENT_DEVICE_BUSY) #define
//// W25QXX_EVENTS_DEVICE_BUSY_RESET(instance)
//// EVENTS_RESET_FLAGS(instance->events, W25QXX_EVENT_DEVICE_BUSY)
//
//namespace wibot::device {
//
//W25qxxSpicmd::W25qxxSpicmd(SpiMasterBus *spi, u32 timeout) : _spi(spi), _timeout(timeout){};
//
//Result W25qxxSpicmd::_spiSendCommand(CommandFrame &frame) {
//    WaitHandler wh  = WaitHandler();
//    auto        rst = _cmdSpi.send(frame, wh);
//    if (rst != Result::kOk) {
//        return rst;
//    }
//    return wh.wait(_timeout);
//};
//
//Result W25qxxSpicmd::reset() {
//    Result rst;
//
//    rst = _resetCommand();
//    if (rst != Result::kOk) {
//        return rst;
//    }
//    rst = _exitQpiCommand();
//    if (rst != Result::kOk) {
//        return rst;
//    }
//    W25qxxStatus2Register reg2;
//    rst = _getStatus(2, reg2.value);
//    if (rst != Result::kOk) {
//        return rst;
//    }
//    reg2.QE = 0;
//    rst     = _setStatus(2, reg2.value);
//    if (rst != Result::kOk) {
//        return rst;
//    }
//    rst = _busyWait();
//    if (rst != Result::kOk) {
//        return rst;
//    }
//
//    _cmdMode = kSpi;
//    return rst;
//};
//
//Result W25qxxSpicmd::switchMode(W25qxxCommandMode cmdMode) {
//    Result rst;
//    if (cmdMode == kSpi) {
//        if (_cmdMode == kQpi) {
//            rst = _exitQpiCommand();
//            if (rst != Result::kOk) {
//                return rst;
//            }
//            _cmdMode = cmdMode;
//        }
//    } else {
//        W25qxxStatus2Register reg2;
//        if (_cmdMode == kSpi) {
//            if (_dummyCycles != 2 && _dummyCycles != 4 && _dummyCycles != 6 && _dummyCycles != 8) {
//                _dummyCycles = 2;
//            }
//            rst = _getStatus(2, reg2.value);
//            if (rst != Result::kOk) {
//                return rst;
//            }
//            reg2.QE = 1;
//            rst     = _setStatus(2, reg2.value);
//            if (rst != Result::kOk) {
//                return rst;
//            }
//            rst = _enterQpiCommand();
//            if (rst != Result::kOk) {
//                return rst;
//            }
//            _cmdMode = cmdMode;
//            rst      = _setReadParameter();
//            if (rst != Result::kOk) {
//                return rst;
//            }
//        }
//    }
//    return Result::kOk;
//};
//
//Result W25qxxSpicmd::eraseBlock(u32 address) {
//    Result rst;
//    rst = _enableWriteCommand();
//    if (rst != Result::kOk) {
//        return rst;
//    }
//
//    rst = _eraseCommand(k4K, address);
//    if (rst != Result::kOk) {
//        return rst;
//    }
//    return _busyWait();
//};
//Result W25qxxSpicmd::eraseChip() {
//    Result rst;
//    rst = _enableWriteCommand();
//    if (rst != Result::kOk) {
//        return rst;
//    }
//    rst = _eraseChipCommand();
//    if (rst != Result::kOk) {
//        return rst;
//    }
//    return _busyWait();
//};
//
//Result W25qxxSpicmd::readMedia(void *data, u32 num, u32 size) {
//    return _readCommand((u8 *)data, num, size);
//};
//Result W25qxxSpicmd::writeMedia(void *data, u32 num, u32 size) {
//    Result rst;
//    do {
//        rst = _enableWriteCommand();
//        if (rst != Result::kOk) {
//            break;
//        }
//
//        rst = _writeCommand((u8 *)data, num, size);
//        if (rst != Result::kOk) {
//            break;
//        }
//        rst = _busyWait();
//    } while (0);
//    return rst;
//};
//
//Result W25qxxSpicmd::eraseMedia(u32 num, u32 size) {
//    Result rst;
//    do {
//        u32 blkBeginAddr = num & ~(kW25qxxBlockSize - 1);
//        u32 blkEndAddr   = (num + size - 1) & ~(kW25qxxBlockSize - 1);
//        u32 curAddr      = blkBeginAddr;
//        do {
//            rst = eraseBlock(curAddr);
//            if (rst != Result::kOk) {
//                break;
//            }
//            curAddr += kW25qxxBlockSize;
//
//        } while (curAddr <= blkEndAddr);
//    } while (0);
//    return rst;
//};
//
//void W25qxxSpicmd::_configCommandLine(CommandFrame &frame, W25qxxCmdLineMode lineMode) {
//    frame.altDataMode = CommandFrameMode::kSkip;
//    switch (lineMode) {
//        case k111:
//            frame.commandMode = CommandFrameMode::k1Line;
//            frame.addressMode = CommandFrameMode::k1Line;
//
//            frame.dataMode = CommandFrameMode::k1Line;
//            break;
//        case k110:
//            frame.commandMode = CommandFrameMode::k1Line;
//            frame.addressMode = CommandFrameMode::k1Line;
//            frame.dataMode    = CommandFrameMode::kSkip;
//            break;
//        case k101:
//            frame.commandMode = CommandFrameMode::k1Line;
//            frame.addressMode = CommandFrameMode::kSkip;
//            frame.dataMode    = CommandFrameMode::k1Line;
//            break;
//        case k100:
//            frame.commandMode = CommandFrameMode::k1Line;
//            frame.addressMode = CommandFrameMode::kSkip;
//            frame.dataMode    = CommandFrameMode::kSkip;
//            break;
//        case k444:
//            frame.commandMode = CommandFrameMode::k4Lines;
//            frame.addressMode = CommandFrameMode::k4Lines;
//            frame.dataMode    = CommandFrameMode::k4Lines;
//            break;
//        case k440:
//            frame.commandMode = CommandFrameMode::k4Lines;
//            frame.addressMode = CommandFrameMode::k4Lines;
//            frame.dataMode    = CommandFrameMode::kSkip;
//            break;
//        case k404:
//            frame.commandMode = CommandFrameMode::k4Lines;
//            frame.addressMode = CommandFrameMode::kSkip;
//            frame.dataMode    = CommandFrameMode::k4Lines;
//            break;
//        case k400:
//            frame.commandMode = CommandFrameMode::k4Lines;
//            frame.addressMode = CommandFrameMode::kSkip;
//            frame.dataMode    = CommandFrameMode::kSkip;
//            break;
//        default:
//            break;
//    }
//    frame.dummyCycles = 0;
//}
//
//Result W25qxxSpicmd::_getStatus(u8 reg_num, u8 &status) {
//    Result       rst;
//    CommandFrame cmd;
//    if (_cmdMode == kQpi) {
//        // QPI
//        _configCommandLine(cmd, k404);
//    } else {
//        // SPI
//        _configCommandLine(cmd, k101);
//    }
//
//    cmd.commandId = (reg_num == 1) ? W25QXX_SPI_READ_STATUS_REG1_CMD
//                                   : ((reg_num == 2) ? W25QXX_SPI_READ_STATUS_REG2_CMD
//                                                     : W25QXX_SPI_READ_STATUS_REG3_CMD);
//    cmd.dataSize  = 1;
//    cmd.dataBits  = DataWidth::k8Bits;
//    cmd.isWrite   = 0;
//    cmd.data      = &status;
//
//    rst = _spiSendCommand(cmd);
//    return rst;
//};
//Result W25qxxSpicmd::_setStatus(u8 reg_num, u8 status) {
//    Result       rst;
//    CommandFrame cmd;
//    if (_cmdMode == kQpi) {
//        // QPI
//        _configCommandLine(cmd, k404);
//    } else {
//        // SPI
//        _configCommandLine(cmd, k101);
//    }
//
//    cmd.commandId = (reg_num == 1) ? W25QXX_SPI_WRITE_STATUS_REG1_CMD
//                                   : ((reg_num == 2) ? W25QXX_SPI_WRITE_STATUS_REG2_CMD
//                                                     : W25QXX_SPI_WRITE_STATUS_REG3_CMD);
//    cmd.dataSize  = 1;
//    cmd.dataBits  = DataWidth::k8Bits;
//    cmd.isWrite   = 1;
//    cmd.data      = &status;
//
//    rst = _spiSendCommand(cmd);
//    return rst;
//};
//
//Result W25qxxSpicmd::readId(u32 &mdId, u32 &jedecId) {
//    Result       rst;
//    CommandFrame cmd;
//    if (_cmdMode == kSpi) {
//        _configCommandLine(cmd, k111);
//    } else {
//        _configCommandLine(cmd, k444);
//    }
//    cmd.commandId   = W25QXX_SPI_READ_ID_CMD;
//    cmd.address     = 0x0000;
//    cmd.addressBits = DataWidth::k16Bits;
//    cmd.data        = &mdId;
//    cmd.dataBits    = DataWidth::k8Bits;
//    cmd.dataSize    = 3;
//    rst             = _spiSendCommand(cmd);
//    if (rst != Result::kOk) {
//        return rst;
//    }
//
//    if (_cmdMode == kSpi) {
//        _configCommandLine(cmd, k101);
//    } else {
//        _configCommandLine(cmd, k404);
//    }
//    cmd.commandId = W25QXX_SPI_READ_JEDEC_ID_CMD;
//    cmd.data      = &jedecId;
//    cmd.dataBits  = DataWidth::k8Bits;
//    cmd.dataSize  = 3;
//    rst           = _spiSendCommand(cmd);
//    return rst;
//};
//Result W25qxxSpicmd::_busyWait() {
//    Result                rst;
//    W25qxxStatus1Register status1;
//    do {
//        rst = _getStatus(1, status1.value);
//
//        if (rst != Result::kOk) {
//            return rst;
//        }
//        if (!status1.BUSY) {
//            return Result::kOk;
//        } else {
//            os::sleep(1);
//        }
//    } while (1);
//};
//
//Result W25qxxSpicmd::_setReadParameter() {
//    Result       rst;
//    CommandFrame cmd;
//    if (_cmdMode == kSpi) {
//        // QPI
//        u8 params;
//        switch (this->_dummyCycles) {
//            case 2:
//                params = 0x00;
//                break;
//            case 4:
//                params = 0x04;
//                break;
//            case 6:
//                params = 0x08;
//                break;
//            case 8:
//                params = 0x0C;
//                break;
//            default:
//                break;
//        }
//
//        _configCommandLine(cmd, k404);
//        cmd.commandId = W25QXX_QPI_SET_READ_PARAMETERS_CMD;
//        cmd.dataSize  = 1;
//        cmd.dataBits  = DataWidth::k8Bits;
//        cmd.isWrite   = 1;
//        cmd.data      = &params;
//        rst           = _spiSendCommand(cmd);
//        return rst;
//    } else {
//        // SPI
//        return Result::kNotSupport;
//    }
//};
//Result W25qxxSpicmd::_enableWriteCommand() {
//    Result       rst;
//    CommandFrame cmd;
//    if (_cmdMode == kSpi) {
//        _configCommandLine(cmd, k100);
//    } else {
//        _configCommandLine(cmd, k400);
//    }
//    cmd.commandId = W25QXX_SPI_WRITE_ENABLE_CMD;
//
//    rst = _spiSendCommand(cmd);
//    return rst;
//};
//
//Result W25qxxSpicmd::_writeCommand(u8 *pData, u32 writeAddr, u32 dataSize) {
//    Result       rst;
//    CommandFrame cmd;
//    if (_cmdMode == kSpi) {
//        _configCommandLine(cmd, k111);
//    } else {
//        _configCommandLine(cmd, k444);
//    }
//
//    cmd.commandId   = W25QXX_SPI_PAGE_PROG_CMD;
//    cmd.address     = writeAddr;
//    cmd.addressBits = DataWidth::k24Bits;
//    cmd.isWrite     = 1;
//    cmd.data        = pData;
//    cmd.dataSize    = dataSize;
//    cmd.dataBits    = DataWidth::k8Bits;
//
//    rst = _spiSendCommand(cmd);
//    return rst;
//};
//Result W25qxxSpicmd::_readCommand(u8 *pData, u32 readAddr, u32 size) {
//    Result       rst;
//    CommandFrame cmd;
//    if (_cmdMode == kSpi) {
//        _configCommandLine(cmd, k111);
//        cmd.commandId   = W25QXX_SPI_READ_CMD;
//        cmd.dummyCycles = 0;
//    } else {
//        _configCommandLine(cmd, k444);
//        cmd.commandId   = W25QXX_QPI_FAST_READ_CMD;
//        cmd.dummyCycles = _dummyCycles;
//    }
//    cmd.address     = readAddr;
//    cmd.addressBits = DataWidth::k24Bits;
//
//    cmd.data     = pData;
//    cmd.dataSize = size;
//    cmd.dataBits = DataWidth::k8Bits;
//    cmd.isWrite  = 0;
//
//    rst = _spiSendCommand(cmd);
//    return rst;
//};
//Result W25qxxSpicmd::_enterQpiCommand() {
//    Result       rst;
//    CommandFrame cmd;
//    _configCommandLine(cmd, k100);
//    cmd.commandId = W25QXX_SPI_ENTER_QPI_MODE_CMD;
//    rst           = _spiSendCommand(cmd);
//    return rst;
//};
//Result W25qxxSpicmd::_exitQpiCommand() {
//    Result       rst;
//    CommandFrame cmd;
//    _configCommandLine(cmd, k400);
//    cmd.commandId = W25QXX_QPI_EXIT_QPI_MODE_CMD;
//    rst           = _spiSendCommand(cmd);
//    return rst;
//};
//Result W25qxxSpicmd::_resetCommand() {
//    Result       rst;
//    CommandFrame cmd;
//    if (_cmdMode == kSpi) {
//        _configCommandLine(cmd, k100);
//    } else {
//        _configCommandLine(cmd, k400);
//    }
//
//    cmd.commandId = W25QXX_QPI_ENABLE_RESET_CMD;
//    rst           = _spiSendCommand(cmd);
//    if (rst != Result::kOk) {
//        return rst;
//    }
//    cmd.commandId = W25QXX_QPI_RESET_DEVICE_CMD;
//    rst           = _spiSendCommand(cmd);
//    return rst;
//};
//
//Result W25qxxSpicmd::_eraseCommand(W25qxxEraseMode mode, u32 address) {
//    Result       rst;
//    CommandFrame cmd;
//    if (_cmdMode == kSpi) {
//        _configCommandLine(cmd, k110);
//    } else {
//        _configCommandLine(cmd, k440);
//    }
//    cmd.commandId   = (mode == k4K) ? W25QXX_SPI_SECTOR_ERASE_4K_CMD
//                                    : ((mode == k32K) ? W25QXX_SPI_BLOCK_ERASE_32K_CMD
//                                                      : W25QXX_SPI_BLOCK_ERASE_64K_CMD);
//    cmd.address     = address;
//    cmd.addressBits = DataWidth::k24Bits;
//
//    rst = _spiSendCommand(cmd);
//    return rst;
//};
//Result W25qxxSpicmd::_eraseChipCommand() {
//    Result       rst;
//    CommandFrame cmd;
//    if (_cmdMode == kSpi) {
//        _configCommandLine(cmd, k100);
//    } else {
//        _configCommandLine(cmd, k440);
//    }
//    cmd.commandId = W25QXX_SPI_CHIP_ERASE_CMD;
//
//    rst = _spiSendCommand(cmd);
//    return rst;
//};
//
//W25qxxSpicmdBlock::W25qxxSpicmdBlock(W25qxxSpicmd &w25qxx, Slice buffer)
//    : Block(buffer), _w25qxx(w25qxx) {
//    setConfig(BlockConfig{
//        .readBlockSize        = 0,
//        .writeBlockSize       = kW25qxxPageSize,
//        .eraseBlockSize       = kW25qxxBlockSize,
//        .readMode             = BlockMode::kRandom,
//        .writeMode            = BlockMode::kWrap,
//        .eraseMode            = BlockMode::kRandomBlock,
//        .needEraseBeforeWrite = true,
//    });
//};
//
//Result W25qxxSpicmdBlock::readMedia(void *data, u32 num, u32 size) {
//    return _w25qxx.readMedia(data, num, size);
//};
//
//Result W25qxxSpicmdBlock::writeMedia(void *data, u32 num, u32 size) {
//    return _w25qxx.writeMedia(data, num, size);
//};
//
//Result W25qxxSpicmdBlock::eraseMedia(u32 num, u32 size) {
//    return _w25qxx.eraseMedia(num, size);
//}
//
//}  // namespace wibot
