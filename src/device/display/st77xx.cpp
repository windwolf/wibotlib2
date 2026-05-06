#include "st77xx.hpp"
#include "type.hpp"

namespace wibot {

St77xx::St77xx(SpiMaster &spi, Pin &dcPin) : _spi(spi), _dcPin(dcPin) {
    SpiConfig cfg = {
        .cpha      = SpiCpha::k1Edge,
        .cpol      = SpiCpol::k0,
        .dataWidth = DataWidth::k8Bits,
    };
    _spi.setConfig(cfg);
};

Result St77xx::sendCommand(u8 cmdId) {
    _dcPin.setValue(0);
    auto rst = _spi.begin();
    if (rst != Result::kOk) {
        return rst;
    }
    auto ar = _spi.write(Slice(&cmdId, 1));
    rst     = ar.wait(TIMEOUT_FOREVER);
    _spi.end();
    return rst;
};

Result St77xx::sendCommandData(u8 cmdId, Slice &data, bool isWrite) {
    _dcPin.setValue(0);
    auto rst = _spi.begin();
    if (rst != Result::kOk) {
        return rst;
    }
    auto ar = _spi.write(Slice(&cmdId, 1));
    rst     = ar.wait(TIMEOUT_FOREVER);
    if (rst != Result::kOk) {
        _spi.end();
        return rst;
    }

    _dcPin.setValue(1);
    if (isWrite) {
        ar = _spi.write(data);
    } else {
        ar = _spi.read(data);
    }
    rst = ar.wait(TIMEOUT_FOREVER);
    _spi.end();
    return rst;
}

Result St77xx::sendReadCommand(u8 cmdId, Slice &data) {
    return sendCommandData(cmdId, data, false);
}

Result St77xx::sendWriteCommand(u8 cmdId, const Slice &data) {
    return sendCommandData(cmdId, const_cast<Slice &>(data), true);
}
}  // namespace wibot
