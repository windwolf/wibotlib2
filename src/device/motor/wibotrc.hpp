#pragma once

#include "bus.hpp"
#include "circular-buffer.hpp"
#include "constant-source.hpp"
#include "crc8.hpp"
#include "dshot.hpp"
#include "message-reader.hpp"
#include "rx-server.hpp"
#include "slope-trajectory.hpp"
#include "type.hpp"
#include "uart.hpp"

#if defined(HAL_TIM_MODULE_ENABLED) && defined(HAL_UART_MODULE_ENABLED)

namespace wibot {
extern "C" struct WibotRcState {
    /**
   * 100 rpm
   */
    uint16_t erpm;
    /**
   * 1 degree. 0-255C
   */
    uint8_t  temperature;
    /**
   * 0.01V
   */
    uint16_t voltage;
    /**
   * 0.01A
   */
    uint16_t current;

    /**
   * mAH
   */
    uint16_t consumption;
};

class WibotRcTelemetry : public RxServer {
   public:
    WibotRcTelemetry(UART_HandleTypeDef& uart);

   public:
    const WibotRcState& getState() const {
        return _state;
    };

   private:
    bool validateFrame(const MessageFrame& frame) override;
    void processCommandFrame(const MessageFrame& frame) override;

   private:
    static constexpr MessageSchema schema = {
        // .prefix            = {0x55},
        .prefixSize        = 0,
        .commandSize       = DataWidth::kNone,
        // .lengthSchemas     = nullptr,
        .lengthSchemaCount = 0,
        .defaultLength{
            .mode  = MessageLengthSchemaMode::kFixedLength,
            .fixed = {.length = 10},
        },
        .alterDataSize = DataWidth::kNone,
        .crcSize       = DataWidth::kNone,
        //.crcRange = kMessageSchemaRangePrefix | kMessageSchemaRangeLength | kMessageSchemaRangeContent,
        // .suffix     = nullptr,
        .suffixSize    = 0};

    Buffer<32>      _msgBuffer;
    CircularBuffer8 _msgCircBuffer{_msgBuffer};
    Uart            _uart;
    MessageReader   _reader{&_uart, _msgCircBuffer, schema, true};
    Crc8Validator   _crcValidator{0x07, 0x00, 0x00, false};
    WibotRcState    _state{};
};

class WibotRcController : public Worker {
   public:
    WibotRcController(TIM_HandleTypeDef& tim, u8 timChannel)
        : _dshot(tim),
          _timChannel(timChannel),
          _throttleSource(0.0f),
          _trajectory(&_throttleSource) {
        _trajectory.setConfig({.slopeRate = 1.0f, .sampleTime = 0.01f});
    };

   public:
    void setThrottle(f32 throttle);

   private:
    void run() override;

   private:
    DShot                  _dshot;
    u8                     _timChannel;
    ConstantSource<f32, 1> _throttleSource;
    SlopeTrajectory<1>     _trajectory;
};

}  // namespace wibot

#endif // HAL_TIM_MODULE_ENABLED && HAL_UART_MODULE_ENABLED
