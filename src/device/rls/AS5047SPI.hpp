#pragma once

//
// Created by zhouj on 2023/2/28.
//

#include "rls.hpp"
#include "bus.hpp"
#include "comm/crc/parity.hpp"

#define AS5047_EF_BIT     0x4000
#define AS5047_PARITY_BIT 0x8000

#define AS5047_CMD_READ_MAG      0x7FFD  // 0x3FFD | AS5047_EF_BIT | 0x0000
#define AS5047_CMD_READ_ANGLEUNC 0x7FFE  // 0x3FFE | AS5047_EF_BIT | 0x0000
#define AS5047_CMD_READ_ANGLECOM 0xFFFF  // 0x3FFF | AS5047_EF_BIT | 0x8000

namespace wibot {

struct As5047SpiConfig {};

struct As5047State {
    bool parityError  : 1;
    bool framingError : 1;
    bool commandError : 1;
    bool invalidData  : 1;
};

/**
 * @note 使用前，确保时钟极性为极性为0，下降沿采样，16位，NSS脉冲模式
 */
class As5047Spi : public Rls {
   public:
    As5047Spi(SpiMaster& spi) : _spi(spi), _angle(0), _state{}, _parity(true) {};

    u32 getAngle() override;

    u32 getData() override;

    As5047State getState() const {
        return _state;
    }

   protected:
    void readCommand(u16 cmd);

   public:
    As5047SpiConfig config;

   private:
    SpiMaster& _spi;
    u16        _cmd[2];

    u16             _angle;
    As5047State     _state;
    ParityValidator _parity;
};

}  // namespace wibot
