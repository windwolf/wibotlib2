//
// Created by zhouj on 2023/9/22.
//

#include "pelco-d.hpp"

namespace wibot::comm {
#define COMMAND1_PTR (reinterpret_cast<Command1*>(&_frame._buffer[2]))
#define COMMAND2_PTR (reinterpret_cast<Command2*>(&_frame._buffer[3]))

PelcoDStandardCommand::PelcoDStandardCommand(PelcoDFrame& frame) : _frame(frame) {
}

PelcoDStandardCommand* PelcoDStandardCommand::sense(bool sence) {
    COMMAND1_PTR->sense = sence;
    return this;
}

PelcoDStandardCommand* PelcoDStandardCommand::setScan(bool isAuto) {
    COMMAND1_PTR->autoManualScan = isAuto;
    return this;
}
PelcoDStandardCommand* PelcoDStandardCommand::setCamera(bool isOn) {
    COMMAND1_PTR->cameraOnOff = isOn;
    return this;
}
PelcoDStandardCommand* PelcoDStandardCommand::setIris(Ternary isOpen) {
    switch (isOpen) {
        case Ternary::kTrue:
            COMMAND1_PTR->irisOpen  = true;
            COMMAND1_PTR->irisClose = false;
            break;
        case Ternary::kFalse:
            COMMAND1_PTR->irisOpen  = false;
            COMMAND1_PTR->irisClose = true;
            break;
        case Ternary::kNone:
            COMMAND1_PTR->irisOpen  = false;
            COMMAND1_PTR->irisClose = false;
            break;
    }

    return this;
}
PelcoDStandardCommand* PelcoDStandardCommand::setFocus(Ternary isNear) {
    switch (isNear) {
        case Ternary::kTrue:
            COMMAND1_PTR->focusNear = true;
            COMMAND2_PTR->focusFar  = false;
            break;
        case Ternary::kFalse:
            COMMAND1_PTR->focusNear = false;
            COMMAND2_PTR->focusFar  = true;
            break;
        case Ternary::kNone:
            COMMAND1_PTR->focusNear = false;
            COMMAND2_PTR->focusFar  = false;
            break;
    }
    return this;
}
PelcoDStandardCommand* PelcoDStandardCommand::setZoom(Ternary isWide) {
    switch (isWide) {
        case Ternary::kTrue:
            COMMAND2_PTR->zoomWide = true;
            COMMAND2_PTR->zoomTele = false;
            break;
        case Ternary::kFalse:
            COMMAND2_PTR->zoomWide = false;
            COMMAND2_PTR->zoomTele = true;
            break;
        case Ternary::kNone:
            COMMAND2_PTR->zoomWide = false;
            COMMAND2_PTR->zoomTele = false;
            break;
    }
    return this;
}
PelcoDStandardCommand* PelcoDStandardCommand::setTilt(i8 speed) {
    if (speed > 64) {
        COMMAND2_PTR->tiltUp   = true;
        COMMAND2_PTR->tiltDown = false;
        _frame._buffer[6]      = 0x3f;
    } else if (speed > 0) {
        COMMAND2_PTR->tiltUp   = true;
        COMMAND2_PTR->tiltDown = false;
        _frame._buffer[6]      = speed - 1;
    } else if (speed == 0) {
        COMMAND2_PTR->tiltUp   = false;
        COMMAND2_PTR->tiltDown = false;
        _frame._buffer[6]      = 0;
    } else if (speed > -64) {
        COMMAND2_PTR->tiltUp   = false;
        COMMAND2_PTR->tiltDown = true;
        _frame._buffer[6]      = -speed - 1;
    } else {
        COMMAND2_PTR->tiltUp   = false;
        COMMAND2_PTR->tiltDown = true;
        _frame._buffer[6]      = 0x3f;
    }

    return this;
}
PelcoDStandardCommand* PelcoDStandardCommand::setPen(i8 speed) {
    if (speed > 64) {
        COMMAND2_PTR->panRight = true;
        COMMAND2_PTR->panLeft  = false;
        _frame._buffer[5]      = 0x40;
    } else if (speed > 0) {
        COMMAND2_PTR->panRight = true;
        COMMAND2_PTR->panLeft  = false;
        _frame._buffer[5]      = speed - 1;
    } else if (speed == 0) {
        COMMAND2_PTR->panRight = false;
        COMMAND2_PTR->panLeft  = false;
        _frame._buffer[5]      = 0;
    } else if (speed > -64) {
        COMMAND2_PTR->panRight = false;
        COMMAND2_PTR->panLeft  = true;
        _frame._buffer[5]      = -speed - 1;
    } else {
        COMMAND2_PTR->panRight = false;
        COMMAND2_PTR->panLeft  = true;
        _frame._buffer[5]      = 0x40;
    }

    return this;
}

}  // namespace wibot
