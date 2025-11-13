#pragma once

//
// Created by zhouj on 2023/10/6.
//

#include "bus.hpp"
#include "chip.hpp"

namespace wibot {

struct I2cTimingConfig {
    u32  frequency;
    u32  timeout;
    bool stretch;
};

/**
 * Timing1:
 *               <  1   > <2> <3>             4   5
 * SCL: ~~~/¯¯¯\ ___/¯¯¯\ ___/¯¯¯\ ___/¯¯¯\ ___/¯¯¯\ ___/¯¯¯\ ___/¯¯¯\ ___/¯¯¯\ \___/¯¯¯~
 *               <  1   > <2> <3>             4   5
 *            6  7 8                     6                                           9
 * SDA: ~/¯¯¯\__ -X------ ~~~~~↓~~ -/¯¯¯\__ -X------ ~~~~~↓~~ ~~~~~↓~~ -X------ --\___/¯~
 *      |<-ST->| |<-WT->| |<-RD->| |<-RS->| |<-WT->| |<-RD->| |<-RD->| |<-WT->| |<-SP->|
 *               7W+1R    8R
 *
 * --------------------------------------------------------------------------------------
 *
 * Timing2:
 *               <  1   >  <2> <3>     4    5
 * SCL: ~~~/¯¯¯  \___/¯¯¯ \___/¯¯¯ \___/¯¯¯ \___/¯¯¯ \___/¯¯¯ \___/¯¯¯ \___/¯¯¯ \___/¯¯¯~
 *            6   7 8                     6                                          9
 * SDA: ~/¯¯¯\_  --X----- ~~~~~~↓~ --/¯¯¯\_ --X----- ~~~~~~↓~ ~~~~~~↓~ --X----- --\___/¯~
 *      |<-ST->| |<-WT->| |<-RD->| |<-RS->| |<-WT->| |<-RD->| |<-RD->| |<-WT->| |<-SP->|
 */
class SoftI2cMaster : public I2cMaster {
   public:
    SoftI2cMaster(GPIO_TypeDef* sclPort, u32 sclPin, GPIO_TypeDef* sdaPort, u32 sdaPin);
    ~SoftI2cMaster();
    Result      setTimingConfig(I2cTimingConfig& config);
    Result      setTransitionConfig(I2cMasterTransitionConfig& config) override;
    AsyncResult readReg(u16 regAddr, const Slice& data) override;
    AsyncResult writeReg(u16 regAddr, const Slice& data) override;
    AsyncResult read(const Slice& data) override;
    AsyncResult write(const Slice& data) override;

   private:
    /**
     * SCL: ~¯¯\_
     * SDA: ~¯\__
     * @return
     */
    Result _i2cStart();

    /**
     * SCL: ~¯¯\_
     * SDA: ~¯\__
     */
    void        _i2cRestart();
    /**
     * SCL: _/¯¯~
     * SDA: __/¯~
     */
    void        _i2cStop();
    /**
     * SCL: ___/¯¯¯\
     * SDA: -X------
     */
    inline void _i2cWriteBit(u8 c);
    /**
     * SCL: ___/¯¯¯\
     * SDA: ~~~~~↓~~
     * @return
     */
    inline u8   _i2cReadBit();
    /**
     *
     * @param c
     * @return 1 = NACK, 0 = ACK
     */
    Result      _i2cWrite(u8 c);
    u8          _i2cRead(bool ack);
    Result      _i2cSendAddress(bool isRead);

   private:
    GPIO_TypeDef* _sclPort;
    u32           _sclPin;
    GPIO_TypeDef* _sdaPort;
    u32           _sdaPin;

    I2cTimingConfig           _baseConfig;
    I2cMasterTransitionConfig _transitionConfig;
    u32                       _i2cDelay;
};

}  // namespace wibot
