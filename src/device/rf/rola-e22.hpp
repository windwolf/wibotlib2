#pragma once

#include "bus.hpp"
#include "hal/stm32/gpio.hpp"

namespace wibot {

enum class RolaE22Mode {
    kTransmition,
    kWor,
    kConfiguration,
    kSleep,
};

enum class RolaE22UartBaudrate : u8 {
    k1200   = 0x00,
    k2400   = 0x20,
    k4800   = 0x40,
    k9600   = 0x60,
    k19200  = 0x80,
    k38400  = 0xA0,
    k57600  = 0xC0,
    k115200 = 0xE0,
};

enum class RolaE22UartParity : u8 {
    kP8N1 = 0x00,
    kP8O1 = 0x08,
    kP8E1 = 0x10,
};

enum class RolaE22Baudrate : u8 {
    k2400  = 0x02,
    k4800  = 0x03,
    k9600  = 0x04,
    k19200 = 0x05,
    k38400 = 0x06,
    k57600 = 0x07,
};

enum class RolaE22Packing : u8 {
    k240 = 0x00,
    k128 = 0x40,
    k64  = 0x80,
    k32  = 0xC0,
};

enum class RolaE22Rssi : u8 {
    kDisable = 0x00,
    kEnable  = 0x20,
};

enum class RolaE22RfPower : u8 {
    k0 = 0x00,  // 22/30/33dbm
    k1 = 0x01,  // 17/27/30dbm
    k2 = 0x02,  // 14/24/27dbm
    k3 = 0x03,  // 11/21/24dbm
};

enum class RolaE22RssiByte : u8 {
    kDisable = 0x00,
    kEnable  = 0x80,
};

enum class RolaE22TransmitionMode : u8 {
    kTransparent = 0x00,
    kFixedport   = 0x40,
};

enum class RolaE22RelayMode : u8 {
    kDisable = 0x00,
    kEnable  = 0x20,
};

enum class RolaE22LbtMode : u8 {
    kDisable = 0x00,
    kEnable  = 0x10,
};

enum class RolaE22WorMode : u8 {
    kReceiver    = 0x00,
    kTransmitter = 0x08,
};

enum class RolaE22WorCycle : u8 {
    k500  = 0x00,
    k1000 = 0x01,
    k1500 = 0x02,
    k2000 = 0x03,
    k2500 = 0x04,
    k3000 = 0x05,
    k3500 = 0x06,
    k4000 = 0x07,
};

struct RolaE22Config {
    u8                     netId;
    u16                    address;
    u16                    channel;  // 410.125MHz + 1MHz * ch
    RolaE22UartBaudrate    uartBaudrate;
    RolaE22UartParity      uartParity;
    RolaE22Baudrate        baudrate;
    RolaE22Packing         packing;
    RolaE22Rssi            rssi;
    RolaE22RfPower         rfPower;
    RolaE22RssiByte        rssiByte;
    RolaE22TransmitionMode transmitionMode;
    RolaE22RelayMode       relayMode;
    RolaE22LbtMode         lbtMode;
    RolaE22WorMode         worMode;
    RolaE22WorCycle        worCycle;

    u16 key;
};

/**
 * @brief ROLA-E22
 * @note Fixedport: transmitionMode=fixedport, data[0]=ADDRH, data[1]=ADDRL, data[2]=CH,
 * @note Broadcast: transmitionMode=transparent,
 * @note Listen: ADDRH=0xFF, ADDRL=0xFF,
 */
class RolaE22Uart {
   public:
    RolaE22Uart(UartStream& uart, Pin& m0, Pin& m1, Pin& aux);

    Result         setConfig(RolaE22Config& config);
    RolaE22Config& getConfig();

    AsyncResult send(const Slice& data);

    AsyncResult receive(Slice& data);

    void setMode(RolaE22Mode mode);

   public:
    void waitAux();

   private:
    UartStream&   _uart;
    RolaE22Config _config;

    Pin&        _m0;
    Pin&        _m1;
    Pin&        _aux;
    RolaE22Mode _mode;
};
}  // namespace wibot
