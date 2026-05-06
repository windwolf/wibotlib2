//
// Created by zhouj on 2023/9/5.
//

#include "rola-e22.hpp"
#
#include "hal/system.hpp"

namespace wibot {

void RolaE22Uart::waitAux() {
    // check and ensure AUX is high.

    while (!_aux.getValue()) {
        sleep(1);
    }
}

void RolaE22Uart::setMode(RolaE22Mode mode) {
    switch (mode) {
        case RolaE22Mode::kTransmition:
            _m0.setValue(false);
            _m1.setValue(false);
            break;
        case RolaE22Mode::kWor:
            _m0.setValue(true);
            _m1.setValue(false);
            break;
        case RolaE22Mode::kConfiguration:
            _m0.setValue(false);
            _m1.setValue(true);
            break;
        case RolaE22Mode::kSleep:
            _m0.setValue(true);
            _m1.setValue(true);
            break;
    }
    _mode = mode;
}
RolaE22Uart::RolaE22Uart(UartStream& uart, Pin& m0, Pin& m1, Pin& aux)
    : _uart(uart), _m0(m0), _m1(m1), _aux(aux), _mode(RolaE22Mode::kTransmition) {
    _config.address         = 0x0000;
    _config.netId           = 0x00;
    _config.uartBaudrate    = RolaE22UartBaudrate::k9600;
    _config.uartParity      = RolaE22UartParity::kP8N1;
    _config.baudrate        = RolaE22Baudrate::k2400;
    _config.packing         = RolaE22Packing::k240;
    _config.rssi            = RolaE22Rssi::kDisable;
    _config.rfPower         = RolaE22RfPower::k0;
    _config.channel         = 0x17;
    _config.rssiByte        = RolaE22RssiByte::kDisable;
    _config.transmitionMode = RolaE22TransmitionMode::kTransparent;
    _config.relayMode       = RolaE22RelayMode::kDisable;
    _config.lbtMode         = RolaE22LbtMode::kDisable;
    _config.worMode         = RolaE22WorMode::kReceiver;
    _config.worCycle        = RolaE22WorCycle::k500;
    _config.key             = 0x00;

    _aux.setConfig(false);

    waitAux();
}

Result RolaE22Uart::setConfig(RolaE22Config& config) {
    u8   cmd[12];
    auto old_mode = _mode;
    waitAux();
    setMode(RolaE22Mode::kConfiguration);

    sleep(1);
    cmd[0] = 0xC0;
    cmd[1] = 0x00;
    cmd[2] = 0x09;
    cmd[3] = (config.address >> 8) & 0xFF;
    cmd[4] = (config.address) & 0xFF;
    cmd[5] = config.netId;
    cmd[6] = (u8)config.uartBaudrate | (u8)config.uartParity | (u8)config.baudrate;
    cmd[7] = (u8)config.packing | (u8)config.rssi | (u8)config.rfPower;
    cmd[8] = (config.channel) & 0xFF;
    cmd[9] = (u8)config.rssiByte | (u8)config.transmitionMode | (u8)config.relayMode |
             (u8)config.lbtMode | (u8)config.worMode | (u8)config.worCycle;
    cmd[10]  = (config.key >> 8) & 0xFF;
    cmd[11]  = (config.key) & 0xFF;
    auto ar  = _uart.write(Slice(cmd, 12));
    auto rst = ar.wait(TIMEOUT_FOREVER);
    waitAux();
    setMode(old_mode);
    waitAux();
    if (rst == Result::kOk) {
        _config = config;
    }
    return rst;
}
AsyncResult RolaE22Uart::send(const Slice& data) {
    setMode(RolaE22Mode::kTransmition);
    waitAux();
    sleep(1);
    return _uart.write(data);
}
AsyncResult RolaE22Uart::receive(Slice& data) {
    setMode(RolaE22Mode::kTransmition);
    waitAux();
    sleep(1);
    return _uart.read(data);
}
RolaE22Config& RolaE22Uart::getConfig() {
    auto old_mode = _mode;
    waitAux();
    setMode(RolaE22Mode::kConfiguration);
    sleep(1);
    waitAux();

    Buffer<3> cmd;
    cmd.data[0] = 0xC1;
    cmd.data[1] = 0x00;
    cmd.data[2] = 0x09;

    Buffer<12> rcv;
    auto       rar = _uart.read(rcv);
    auto       war = _uart.write(cmd);
    war.wait(TIMEOUT_FOREVER);
    rar.wait(TIMEOUT_FOREVER);
    _config.address         = rcv.data[3] << 8 | rcv.data[4];
    _config.netId           = rcv.data[5];
    _config.uartBaudrate    = (RolaE22UartBaudrate)(rcv.data[6] & 0xE0);
    _config.uartParity      = (RolaE22UartParity)(rcv.data[6] & 0x18);
    _config.baudrate        = (RolaE22Baudrate)(rcv.data[6] & 0x07);
    _config.packing         = (RolaE22Packing)(rcv.data[7] & 0xC0);
    _config.rssi            = (RolaE22Rssi)(rcv.data[7] & 0x20);
    _config.rfPower         = (RolaE22RfPower)(rcv.data[7] & 0x03);
    _config.channel         = rcv.data[8];
    _config.rssiByte        = (RolaE22RssiByte)(rcv.data[9] & 0x80);
    _config.transmitionMode = (RolaE22TransmitionMode)(rcv.data[9] & 0x40);
    _config.relayMode       = (RolaE22RelayMode)(rcv.data[9] & 0x20);
    _config.lbtMode         = (RolaE22LbtMode)(rcv.data[9] & 0x10);
    _config.worMode         = (RolaE22WorMode)(rcv.data[9] & 0x08);
    _config.worCycle        = (RolaE22WorCycle)(rcv.data[9] & 0x07);
    _config.key             = rcv.data[10] << 8 | rcv.data[11];

    setMode(old_mode);
    sleep(1);
    waitAux();
    return _config;
}

}  // namespace wibot
