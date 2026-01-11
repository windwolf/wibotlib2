#pragma once

#include "chip.hpp"
#include "bus.hpp"
#include "circular-buffer.hpp"
#include "comm/crc/crc8.hpp"
#include "comm/protocol/dshot.hpp"
#include "comm/msg/message-reader.hpp"
#include "rx-server.hpp"
#include "dsp/controller/slope.hpp"
#include "type.hpp"
#include "hal/stm32/uart.hpp"

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
        : _dshot(tim), _timChannel(timChannel) {};

   public:
    void setThrottle(f32 throttle);

   private:
    void run() override;

   private:
    DShot                        _dshot;
    u8                           _timChannel;
    f32                          _throttle;
    f32                          _slopedThrottle{0.0f};
    SlopeTrajectory<f32>::Config _slopeConfig{.slopeRate   = 1.0f,  // 1 unit per second
                                              .enableClamp = false,

                                              .minValue = 0.0f,
                                              .maxValue = 1.0f};
    SlopeTrajectory<f32>         _trajectory{_slopeConfig};
};

}  // namespace wibot

#endif  // HAL_TIM_MODULE_ENABLED && HAL_UART_MODULE_ENABLED
