#include "chrono.hpp"

#include "logger.hpp"
LOGGER("chrono")

namespace wibot {

static u32 DateToDays(u8 year, u8 month, u8 day) {
    u8  a = (14 - month) / 12;
    u8  y = year - a;
    u32 m = month + 12 * a - 3;

    return day + (153 * m + 2) / 5 + 365 * y + y / 4 - y / 100 + y / 400 - 30;
};

Date::Date() : year(0), month(1), day(1) {};
Date::Date(u8 year, u8 month, u8 day) : year(year), month(month), day(day) {};

i32 Date::getSpan(const Date &end, ChronoUnitType unitType) const {
    i32 rst = DateToDays(end.year, end.month, end.day) - DateToDays(year, month, day);
    if (unitType == ChronoUnitType::kDay) {
        return rst;
    }
    rst = rst * 24;
    if (unitType == ChronoUnitType::kHour) {
        return rst;
    }
    rst = rst * 60;
    if (unitType == ChronoUnitType::kMinute) {
        return rst;
    }
    rst = rst * 60;
    if (unitType == ChronoUnitType::kSecond) {
        return rst;
    }
    return -1;
};
u32 Date::toNumber(ChronoUnitType unitType) const {
    u32 rst = DateToDays(year, month, day);
    if (unitType == ChronoUnitType::kDay) {
        return rst;
    }
    rst = rst * 24;
    if (unitType == ChronoUnitType::kHour) {
        return rst;
    }
    rst = rst * 60;
    if (unitType == ChronoUnitType::kMinute) {
        return rst;
    }
    rst = rst * 60;
    if (unitType == ChronoUnitType::kSecond) {
        return rst;
    }
    return -1;
};
bool Date::equals(const Date &other, ChronoUnitType unitType) const {
    return year == other.year && month == other.month && day == other.day;
};

Time::Time() : hour(0), minute(0), second(0) {};
Time::Time(u8 hour, u8 minute, u8 second) : hour(hour), minute(minute), second(second) {};

i32 Time::getSpan(const Time &end, ChronoUnitType unitType) const {
    i32 rst = 0;
    if (unitType == ChronoUnitType::kDay) {
        return 0;
    }
    rst = end.hour - hour;
    if (unitType == ChronoUnitType::kHour) {
        return rst;
    }
    rst = rst * 60 + end.minute - minute;
    if (unitType == ChronoUnitType::kMinute) {
        return rst;
    }
    rst = rst * 60 + end.second - second;
    if (unitType == ChronoUnitType::kSecond) {
        return rst;
    }
    return -1;
};
u32 Time::toNumber(ChronoUnitType unitType) const {
    u32 rst = 0;
    if (unitType == ChronoUnitType::kDay) {
        return rst;
    }
    rst = hour;
    if (unitType == ChronoUnitType::kHour) {
        return rst;
    }
    rst = rst * 60 + minute;
    if (unitType == ChronoUnitType::kMinute) {
        return rst;
    }
    rst = rst * 60 + second;
    if (unitType == ChronoUnitType::kSecond) {
        return rst;
    }
    return -1;
};
bool Time::equals(const Time &other, ChronoUnitType unitType) const {
    ASSERT(unitType != ChronoUnitType::kDay, "unsupported unit type: kDay");
    bool rst = hour == other.hour;
    if (unitType == ChronoUnitType::kHour) {
        return rst;
    }
    rst = rst && (minute == other.minute);
    if (unitType == ChronoUnitType::kMinute) {
        return rst;
    }
    rst = rst && (second == other.second);
    if (unitType == ChronoUnitType::kSecond) {
        return rst;
    }

    return false;
};

DateTime::DateTime() : year(0), month(1), day(1), hour(0), minute(0), second(0) {
}
DateTime::DateTime(u8 year, u8 month, u8 day, u8 hour, u8 minute, u8 second)
    : year(year), month(month), day(day), hour(hour), minute(minute), second(second) {
}

i32 DateTime::getSpan(const DateTime &end, ChronoUnitType unitType) const {
    i32 rst = DateToDays(end.year, end.month, end.day) - DateToDays(year, month, day);
    if (unitType == ChronoUnitType::kDay) {
        return rst;
    }
    rst = rst * 24 + end.hour - hour;
    if (unitType == ChronoUnitType::kHour) {
        return rst;
    }
    rst = rst * 60 + end.minute - minute;
    if (unitType == ChronoUnitType::kMinute) {
        return rst;
    }
    rst = rst * 60 + end.second - second;
    if (unitType == ChronoUnitType::kSecond) {
        return rst;
    }
    return -1;
};
u32 DateTime::toNumber(ChronoUnitType unitType) const {
    u32 rst = DateToDays(year, month, day);
    if (unitType == ChronoUnitType::kDay) {
        return rst;
    }
    rst = rst * 24 + hour;
    if (unitType == ChronoUnitType::kHour) {
        return rst;
    }
    rst = rst * 60 + minute;
    if (unitType == ChronoUnitType::kMinute) {
        return rst;
    }
    rst = rst * 60 + second;
    if (unitType == ChronoUnitType::kSecond) {
        return rst;
    }
    return -1;
};
bool DateTime::equals(const DateTime &other, ChronoUnitType unitType) const {
    bool rst = year == other.year && month == other.month && day == other.day;
    if (unitType == ChronoUnitType::kDay) {
        return rst;
    }
    rst = rst && (hour == other.hour);
    if (unitType == ChronoUnitType::kHour) {
        return rst;
    }
    rst = rst && (minute == other.minute);
    if (unitType == ChronoUnitType::kMinute) {
        return rst;
    }
    rst = rst && (second == other.second);
    if (unitType == ChronoUnitType::kSecond) {
        return rst;
    }
    return false;
}
const DateTime &DateTime::getAD0() {
    static const DateTime AD0(0, 1, 1, 0, 0, 0);
    return AD0;
};
}  // namespace wibot
