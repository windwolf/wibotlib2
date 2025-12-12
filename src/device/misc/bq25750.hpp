#pragma once

#include "bus.hpp"

namespace wibot {
class Bq25750 {
   public:
    enum struct ChargeStat : u8 {
        NoChange              = 0b000,
        TrickleCharge         = 0b001,
        PreCharge             = 0b010,
        FastCharge            = 0b011,
        TaperCharge           = 0b100,
        Reserved              = 0b101,
        TopoffTimerCharge     = 0b110,
        ChargeTerminationDone = 0b111,
    };
    enum struct MPPTStat : u8 {
        Disable                 = 0b00,
        EnableNotRunning        = 0b01,
        FullPanelSweep          = 0b10,
        MaxPowerVoltageDetected = 0b11,
    };
    enum struct TSStatus : u8 {
        Normal = 0b000,
        Warm   = 0b001,
        Cool   = 0b010,
        Cold   = 0b011,
        Hot    = 0b100,
    };
    enum struct FswSyncStatus : u8 {
        Normal   = 0b00,
        ExtClock = 0b01,
        PinFault = 0b10,
        Reserved = 0b11,
    };

    enum struct DeadTime : u8 {
        k45ns  = 0b00,
        k75ns  = 0b01,
        k105ns = 0b10,
        k135ns = 0b11,
    };
    enum struct DriverStrength : u8 {
        kFastist = 0b00,
        kFast    = 0b01,
        kSlow    = 0b10,
        kSlowest = 0b11,
    };
    struct State {
        union {
            struct {
                ChargeStat chargeStat             : 3;
                bool       watchDogTimerExpired   : 1;
                u8         _res                   : 1;
                bool       inputVoltageRegulation : 1;
                bool       inputCurrentRegulation : 1;
                bool       adcConversionComplete  : 1;
            };
            u8 raw;
        } chargerStatus1;
        union {
            struct {
                MPPTStat mpptStat  : 2;
                u8       _res      : 2;
                TSStatus tsStatus  : 3;
                bool     powerGood : 1;
            };
            u8 raw;
        } chargerStatus2;
        union {
            struct {
                bool          batfetOn       : 1;
                bool          acfetOn        : 1;
                bool          reverseModeOn  : 1;
                bool          cvTimerExpired : 1;
                FswSyncStatus fswSyncStatus  : 2;
                u8            _res           : 2;
            };
            u8 raw;
        } chargerStatus3;
        union {
            struct {
                u8   _res                     : 1;
                bool drvsupOutOfRange         : 1;
                bool chargeSafetyTimerExpired : 1;
                bool thermalShutdownActive    : 1;
                bool batteryOverVoltage       : 1;
                bool batteryOverCurrent       : 1;
                bool inputOverVoltage         : 1;
                bool inputUnderVoltage        : 1;
            };
            u8 raw;
        } faultStatus;
    };

    struct IntFlag {
        union {
            struct {
                bool charge  : 1;
                bool cvTimer : 1;
                u8   _res1   : 1;
                bool wd      : 1;
                bool _res2   : 1;
                bool vacDpm  : 1;
                bool iacDpm  : 1;
                bool adcDone : 1;
            };
            u8 raw;
        } intFlag1;
        union {
            struct {
                bool mppt          : 1;
                bool fswSyncStatus : 1;
                u8   _             : 1;
                bool reverse       : 1;
                bool ts            : 1;
                bool batfet        : 1;
                bool acfet         : 1;
                bool pg            : 1;
            };
            u8 raw;
        } intFlag2;
        union {
            struct {
                u8   _           : 1;
                bool drvOkz      : 1;
                bool chargeTimer : 1;
                bool ts          : 1;
                bool vbatOv      : 1;
                bool ibatOcp     : 1;
                bool vacOv       : 1;
                bool vacUv       : 1;
            };
            u8 raw;
        } faultFlag;
    };

    struct IntMask {
        union {
            struct {
                bool charge  : 1;
                bool cvTimer : 1;
                u8   _res1   : 1;
                bool wd      : 1;
                bool _res2   : 1;
                bool vacDpm  : 1;
                bool iacDpm  : 1;
                bool adcDone : 1;
            };
            u8 raw;
        } intMask1;
        union {
            struct {
                bool mppt          : 1;
                bool fswSyncStatus : 1;
                u8   _res1         : 1;
                bool reverse       : 1;
                bool ts            : 1;
                bool batfet        : 1;
                bool acfet         : 1;
                bool pg            : 1;
            };
            u8 raw;
        } intMask2;
        union {
            struct {
                u8   _res1       : 1;
                bool drvOkz      : 1;
                bool chargeTimer : 1;
                bool ts          : 1;
                bool vbatOv      : 1;
                bool ibatOcp     : 1;
                bool vacOv       : 1;
                bool vacUv       : 1;
            };
            u8 raw;
        } faultMask;
    };

    struct GateDriverControl {
        union {
            struct {
                DriverStrength buck  : 2;
                DriverStrength boost : 2;
                u8             _res  : 4;
            };
            u8 raw;
        } driverStrength;
        union {
            struct {
                DeadTime buck  : 2;
                DeadTime boost : 2;
                u8       _res  : 4;
            };
            u8 raw;
        } deatTime;
    };

    static const u8 I2C_ADDRESS = 0x6B;

   public:
    Bq25750(I2cMaster& i2c);
    ~Bq25750();

    State getState();

    IntFlag getFlag();

    void setMask(IntMask mask);

    Result enableCharging();
    Result disableCharging();

    Result setGateDriver(GateDriverControl control);

    Result feedDog();

   private:
    I2cMaster& _i2c;
};
}  // namespace wibot
