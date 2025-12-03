#include "wibotrc.hpp"
#include "os.hpp"
#include "type.hpp"

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
    : RxServer(_reader), _uart{uart, "escrx", _msgCircBuffer} {};

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

void WibotRcController::setThrottle(f32 throttle) {
    _throttleSource.setValue(throttle);
};

void WibotRcController::run() {
    while (true) {
        _trajectory.update();
        auto throttle     = _trajectory.getValue();
        auto dshotCommand = static_cast<u16>(throttle * 2000.0f + 48.0f);
        auto rst          = _dshot.send(_timChannel, dshotCommand, false);
        rst.wait(TIMEOUT_NOWAIT);
        os::sleep(1);
    }
}

}  // namespace wibot

#endif  // HAL_TIM_MODULE_ENABLED && HAL_UART_MODULE_ENABLED