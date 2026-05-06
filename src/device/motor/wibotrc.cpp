#include "wibotrc.hpp"
#include "os/os.hpp"
#include "type.hpp"
#include "hal/system.hpp"

#if defined(HAL_TIM_MODULE_ENABLED) && defined(HAL_UART_MODULE_ENABLED)

namespace wibot {

typedef struct __attribute__((packed)) {
    uint8_t temperature;  // temperature in Celcius
    uint8_t voltage_h;    // voltage in centivolts
    uint8_t voltage_l;
    uint8_t current_h;  // current in centiamps
    uint8_t current_l;
    uint8_t consumption_h;  // accumulated current consumption in mAH
    uint8_t consumption_l;
    uint8_t erpm_h;  // eRPM * 100, so 1 in the packet means 100 eRPM
    uint8_t erpm_l;
    uint8_t crc;

} kiss_telem_pkt_t;

WibotRcTelemetry::WibotRcTelemetry(UART_HandleTypeDef& uart)
    : RxServer(_reader, TIMEOUT_FOREVER), _uart{uart, "escrx", &_msgCircBuffer} {};

bool WibotRcTelemetry::validateFrame(const MessageFrame& frame) {
    _crcValidator.reset();
    auto frameData = frame.getWholeBuffer();
    _crcValidator.calculate(frameData.data, sizeof(kiss_telem_pkt_t) - 1);

    return _crcValidator.validate(frameData.data + sizeof(kiss_telem_pkt_t) - 1);
};
void WibotRcTelemetry::processCommandFrame(const MessageFrame& frame) {
    auto buf           = reinterpret_cast<kiss_telem_pkt_t*>(frame.getWholeBuffer().data);
    _state.temperature = buf->temperature;                              // temperature in Celcius
    _state.erpm        = buf->erpm_h << 8 | buf->erpm_l;                // eRPM * 100
    _state.voltage     = buf->voltage_h << 8 | buf->voltage_l;          // voltage in centivolts
    _state.current     = buf->current_h << 8 | buf->current_l;          // current in centiamps
    _state.consumption = buf->consumption_h << 8 | buf->consumption_l;  // mAH
};

void WibotRcController::init() {
    _throttle       = 0;
    _command        = WibotRcCommand::kStop;
    _slopedThrottle = 0;
    _trajectory.setInitialValue(0);
    _lastUpdateTick = System::getTickMs();
};

void WibotRcController::doLoop() {
    u16 dShotCommand;
    if (_command != WibotRcCommand::kNone) {
        dShotCommand = static_cast<u16>(_command);
    } else {
        auto tick       = System::getTickMs();
        _slopedThrottle = _trajectory.update(_throttle, tick - _lastUpdateTick);
        _lastUpdateTick = tick;
        dShotCommand    = static_cast<u16>(
            _slopedThrottle +
            47);  //实测发现+48后电机会有轻微的抖动，可能是因为接近DShot的最小有效命令47，所以这里取48-1=47来避免抖动
    }

    auto rst = _dshot.send(_timChannel, dShotCommand, false);
    rst.wait(TIMEOUT_NOWAIT);
};

}  // namespace wibot

#endif  // HAL_TIM_MODULE_ENABLED && HAL_UART_MODULE_ENABLED
