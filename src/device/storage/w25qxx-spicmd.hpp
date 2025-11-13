#pragma once

////
//// Created by zhouj on 2023/9/19.
////
//
//#ifndef WIBITLIB_DEVICE_STORAGE_W25QXX_CMD_HPP_
//#define WIBITLIB_DEVICE_STORAGE_W25QXX_CMD_HPP_
//
//#include "w25qxx-spi.hpp"
//
//namespace wibot {
//
//class W25qxxSpicmd {
//   public:
//    W25qxxSpicmd(SpiMasterBus *spi, u32 timeout);
//
//    Result reset();
//
//    Result switchMode(W25qxxCommandMode cmdMode);
//
//    Result eraseBlock(u32 address);
//    Result eraseChip();
//    Result readId(u32 &mdId, u32 &jedecId);
//
//    Result readMedia(void *data, u32 num, u32 size);
//    Result writeMedia(void *data, u32 num, u32 size);
//    Result eraseMedia(u32 num, u32 size);
//
//   private:
//    Result _getStatus(u8 reg_num, u8 &status);
//    Result _setStatus(u8 reg_num, u8 status);
//
//    Result      _busyWait();
//    Result      _setReadParameter();
//    static void _configCommandLine(CommandFrame &frame, W25qxxCmdLineMode lineMode);
//
//    Result _enableWriteCommand();
//    Result _writeCommand(u8 *pData, u32 writeAddr, u32 dataSize);
//    Result _readCommand(u8 *pData, u32 readAddr, u32 size);
//    Result _eraseCommand(W25qxxEraseMode mode, u32 address);
//    Result _eraseChipCommand();
//    Result _enterQpiCommand();
//    Result _exitQpiCommand();
//    Result _resetCommand();
//    Result _spiSendCommand(CommandFrame &frame);
//
//   private:
//    SpiMasterBus     *_spi;
//    u32          _timeout;
//    W25qxxCommandMode _cmdMode;
//    u8           _dummyCycles;
//};
//
//class W25qxxSpicmdBlock : public Block {
//   public:
//    W25qxxSpicmdBlock(W25qxxSpicmd &w25qxx, Slice buffer);
//
//   protected:
//    Result readMedia(void *data, u32 num, u32 size) override;
//    Result writeMedia(void *data, u32 num, u32 size) override;
//    Result eraseMedia(u32 num, u32 size) override;
//
//   protected:
//    W25qxxSpicmd &_w25qxx;
//};
//
//}  // namespace wibot
//
//#endif  //WIBITLIB_DEVICE_STORAGE_W25QXX_CMD_HPP_
