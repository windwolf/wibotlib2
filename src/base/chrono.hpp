#pragma once

#include "type.hpp"

namespace wibot {
enum class ChronoUnitType {
    kSecond = 0,
    kMinute = 1,
    kHour   = 2,
    kDay    = 3,
    //    kWeek= 4,
    //    kMonth= 5,
    //    kYear= 6,
};
struct Date {
    u8 year;
    u8 month;
    u8 day;

    Date();
    Date(u8 year, u8 month, u8 day);
    i32  getSpan(const Date &end, ChronoUnitType unitType) const;
    u32  toNumber(ChronoUnitType unitType) const;
    bool equals(const Date &other, ChronoUnitType unitType) const;
};

struct Time {
    u8 hour;
    u8 minute;
    u8 second;
    Time();
    Time(u8 hour, u8 minute, u8 second);

    i32  getSpan(const Time &end, ChronoUnitType unitType) const;
    u32  toNumber(ChronoUnitType unitType) const;
    bool equals(const Time &other, ChronoUnitType unitType) const;
};

struct DateTime {
    u8 year;
    u8 month;
    u8 day;
    u8 hour;
    u8 minute;
    u8 second;
    DateTime();
    DateTime(u8 year, u8 month, u8 day, u8 hour, u8 minute, u8 second);

    i32  getSpan(const DateTime &end, ChronoUnitType unitType) const;
    u32  toNumber(ChronoUnitType unitType) const;
    bool equals(const DateTime &other, ChronoUnitType unitType) const;

    static const DateTime &getAD0();
};

}  // namespace wibot
