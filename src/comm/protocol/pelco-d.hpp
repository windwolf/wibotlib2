#pragma once

//
// Created by zhouj on 2023/9/22.
//

#include "../msg/message-parser.hpp"
namespace wibot {

class PelcoDFrame {
   public:
    Slice getBuffer(u32 address);

   private:
    friend class PelcoDStandardCommand;
    friend class PelcoDExtendedCommand;
    u8 _buffer[7];
};

class PelcoDStandardCommand {
   public:
    explicit PelcoDStandardCommand(PelcoDFrame& frame);
    PelcoDStandardCommand* sense(bool sence);
    PelcoDStandardCommand* setScan(bool isAuto);
    PelcoDStandardCommand* setCamera(bool isOn);
    PelcoDStandardCommand* setIris(Ternary isOpen);   // open / close
    PelcoDStandardCommand* setFocus(Ternary isNear);  // near / far
    PelcoDStandardCommand* setZoom(Ternary isWide);   //wide / tele
    /**
     * speed > 64        => up: limit to max speed
     * 0 > speed >= 64   => up: normal
     * speed = 0         => stop
     * 0 >= speed > -64  => down: normal
     * -64 > speed       => down: limit to max speed
     * @param speed
     * @return
     */
    PelcoDStandardCommand* setTilt(i8 speed);  //+: up, -:down

    /**
     * speed > 64        => right: turbo
     * 0 > speed >= 64   => right: normal
     * speed = 0         => stop
     * 0 >= speed > -64  => left: normal
     * -64 > speed       => left: turbo
     * @param speed
     * @return
     */
    PelcoDStandardCommand* setPen(i8 speed);  //+: right, -:left

   private:
    struct Command1 {
        bool sense          : 1;
        u8                  : 2;
        bool autoManualScan : 1;  //auto / manual
        bool cameraOnOff    : 1;  // on / off
        bool irisClose      : 1;
        bool irisOpen       : 1;
        bool focusNear      : 1;
    };
    struct Command2 {
        bool focusFar : 1;
        bool zoomWide : 1;  //auto / manual
        bool zoomTele : 1;  // on / off
        bool tiltDown : 1;
        bool tiltUp   : 1;
        bool panLeft  : 1;
        bool panRight : 1;
        bool fixed    : 1 = false;
    };

   private:
    PelcoDFrame& _frame;
};

class PelcoDExtendedCommand {
   public:
    PelcoDExtendedCommand(PelcoDFrame& frame);

   private:
    PelcoDFrame& _frame;
};

class PelcoD {
    constexpr static MessageSchema schema = {
        .prefix            = {0xFF},
        .prefixSize        = 1,
        .commandSize       = DataWidth::kNone,
        .lengthSchemas     = nullptr,
        .lengthSchemaCount = 0,
        .defaultLength     = {},
        .alterDataSize     = DataWidth::kNone,
        .crcSize           = DataWidth::k8Bits,
        .crcRange          = kMessageSchemaRangeContent,
        .suffix            = {},
        .suffixSize        = 0,
    };

};  // namespace wibot

}  // namespace wibot

