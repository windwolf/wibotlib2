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
#include "control-loop.hpp"

#if defined(HAL_TIM_MODULE_ENABLED) && defined(HAL_UART_MODULE_ENABLED)

namespace wibot {
extern "C" struct WibotRcState {
    /**
   * 100 rpm
   */
    u16 erpm;
    /**
   * 1 degree. 0-255C 
   */
    u8  temperature;
    /**
   * 0.01V
   */
    u16 voltage;
    /**
   * 0.01A
   */
    u16 current;

    /**
   * mAH
   */
    u16 consumption;
};

class WibotRcTelemetry : public RxServer {
   public:
    /**
    * @brief Construct a new Wibot Rc Telemetry object
    * 
    * @param uart 115200 baud, 8N1, no flow control
    */
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

enum class WibotRcCommand : u8 {
    kNone = 0xFF,
    kStop = 0,
};

class WibotRcController : public EventDrivenControlLoop {
   public:
    WibotRcController(TIM_HandleTypeDef& tim, u8 timChannel, u16 slopeRate = 2000 /* unit/s */)
        : _dshot(tim, DShotProtocol::DShot600),
          _timChannel(timChannel),
          _slopeConfig{
              .slopeRate = slopeRate, .enableClamp = false, .minValue = 0, .maxValue = 2000},
          _trajectory(_slopeConfig) {};

   public:
    /**
    * @brief Set the Throttle object
    * 
    * @param throttle 0.0 - 1.0
    */
    void setThrottle(u16 throttle) {
        _command = WibotRcCommand::kNone;
        _throttle = throttle;

    };
    /**
     * @brief Set the Throttle object
     * 
     * @param throttle 0 - 2000
     */
    void setThrottle(f32 throttle) {
        _command = WibotRcCommand::kNone;
        _throttle = throttle * 2000;  // scale to 0-2000 for DShot command
    };

    void setCommand(WibotRcCommand cmd) {
        _command  = cmd;
        _throttle = 0;
    };

    u16 getSlopedThrottle() const {
        return _slopedThrottle;
    };

    void init() override;
    void doLoop() override;

   private:
    DShot                            _dshot;
    u8                               _timChannel;
    u16                              _throttle;
    WibotRcCommand                               _command;
    u16                              _slopedThrottle;
    SlopeTrajectoryFast<u16>::Config _slopeConfig;
    SlopeTrajectoryFast<u16>         _trajectory;

    u32 _lastUpdateTick = 0;
};

}  // namespace wibot

#endif  // HAL_TIM_MODULE_ENABLED && HAL_UART_MODULE_ENABLED
