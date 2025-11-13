# NMEA 解析器时间日期类型整合总结

## 更新概述

将 NMEA 协议解析器中的自定义 `Time` 和 `Date` 结构替换为项目标准的 `wibot::Time` 和 `wibot::Date` 类，实现时间日期处理的统一化。

## 主要更改

### 1. 头文件更新 (`nmea.hpp`)

#### 添加依赖
```cpp
#include "../base/chrono.hpp"  // 引入标准时间日期类
```

#### 类型别名定义
```cpp
// 使用 wibot::Time 和 wibot::Date 来代替自定义的时间日期结构
using NmeaTime = wibot::Time;
using NmeaDate = wibot::Date;
```

#### 移除重复定义
删除了原有的自定义 `Time` 和 `Date` 结构：
```cpp
// 已移除
struct Date {
    i16 year{-1};
    i8  month{-1};
    i8  day{-1};
    // ...
};

struct Time {
    i8  hours{-1};
    i8  minutes{-1};
    i8  seconds{-1};
    i32 microseconds{-1};
    // ...
};
```

### 2. 数据结构更新

所有 NMEA 数据结构现在使用统一的时间日期类型：

```cpp
struct RmcData {
    NmeaTime time;    // 替代 Time time;
    NmeaDate date;    // 替代 Date date;
    // ...
};

struct GgaData {
    NmeaTime time;    // 替代 Time time;
    // ...
};

// 类似更新应用于 GllData, GstData, ZdaData
```

### 3. 解析函数签名更新

```cpp
class SentenceUtils {
    // 更新前
    [[nodiscard]] static Time parseTime(std::string_view field) noexcept;
    [[nodiscard]] static Date parseDate(std::string_view field) noexcept;
    
    // 更新后
    [[nodiscard]] static NmeaTime parseTime(std::string_view field) noexcept;
    [[nodiscard]] static NmeaDate parseDate(std::string_view field) noexcept;
};
```

### 4. 实现文件更新 (`nmea.cpp`)

#### parseTime 函数重构
```cpp
// 更新前 - 使用自定义 Time 结构
Time SentenceUtils::parseTime(std::string_view field) noexcept {
    Time time;
    time.hours = static_cast<i8>(parseInt(hours_str));
    time.minutes = static_cast<i8>(parseInt(minutes_str));
    time.seconds = static_cast<i8>(parseInt(seconds_str));
    time.microseconds = microseconds;  // 支持微秒
    return time;
}

// 更新后 - 使用 wibot::Time
NmeaTime SentenceUtils::parseTime(std::string_view field) noexcept {
    const u8 hours   = static_cast<u8>(parseInt(hours_str));
    const u8 minutes = static_cast<u8>(parseInt(minutes_str));
    const u8 seconds = static_cast<u8>(parseInt(seconds_str));
    
    // 注意：wibot::Time 没有 microseconds 字段
    return NmeaTime{hours, minutes, seconds};
}
```

#### parseDate 函数重构
```cpp
// 更新前 - 使用自定义 Date 结构，支持完整年份
Date SentenceUtils::parseDate(std::string_view field) noexcept {
    Date date;
    date.year = static_cast<i16>(full_year);  // 支持4位年份
    date.month = static_cast<i8>(month);
    date.day = static_cast<i8>(day);
    return date;
}

// 更新后 - 使用 wibot::Date，年份限制为u8
NmeaDate SentenceUtils::parseDate(std::string_view field) noexcept {
    const u8 year = static_cast<u8>(parseInt(field.substr(4, 2)));
    const u8 month = static_cast<u8>(parseInt(field.substr(2, 2)));
    const u8 day = static_cast<u8>(parseInt(field.substr(0, 2)));
    
    // 年份策略：保持2位年份格式，80-99表示1980-1999，0-79表示2000-2079
    return NmeaDate{year, month, day};
}
```

#### ZDA 解析器特殊处理
```cpp
// ZDA 提供4位年份，需要转换为 u8 格式
const i32 full_year = SentenceUtils::parseInt(fields.next());
const u8 year = static_cast<u8>(full_year >= 2000 ? full_year - 2000 : 0);
zda_data->date = NmeaDate{year, month, day};
```

## 字段名称映射

### wibot::Time vs 原 NMEA Time
| wibot::Time | 原 NMEA Time   | 类型变化     | 说明                               |
| ----------- | -------------- | ------------ | ---------------------------------- |
| `hour`      | `hours`        | `u8` vs `i8` | 字段名去除复数形式，类型改为无符号 |
| `minute`    | `minutes`      | `u8` vs `i8` | 字段名去除复数形式，类型改为无符号 |
| `second`    | `seconds`      | `u8` vs `i8` | 字段名去除复数形式，类型改为无符号 |
| *缺失*      | `microseconds` | -            | wibot::Time 不支持微秒精度         |

### wibot::Date vs 原 NMEA Date
| wibot::Date | 原 NMEA Date | 类型变化      | 说明                           |
| ----------- | ------------ | ------------- | ------------------------------ |
| `year`      | `year`       | `u8` vs `i16` | 年份范围限制，需要特殊编码策略 |
| `month`     | `month`      | `u8` vs `i8`  | 类型改为无符号                 |
| `day`       | `day`        | `u8` vs `i8`  | 类型改为无符号                 |

## 兼容性考虑

### 1. 微秒精度丢失
- **问题**：wibot::Time 不支持微秒字段
- **影响**：NMEA 时间戳的微秒信息会丢失
- **解决方案**：如果需要微秒精度，考虑：
  - 扩展 wibot::Time 结构
  - 使用自定义时间结构
  - 在应用层保存微秒信息

### 2. 年份范围限制
- **问题**：wibot::Date 使用 u8 年份，范围 0-255
- **当前策略**：
  - 2位年份：80-99 映射到 1980-1999
  - 2位年份：00-79 映射到 2000-2079
  - 4位年份：减去2000年作为偏移量
- **影响**：无法表示2255年以后的日期

### 3. 字段访问变化
- **字段名**：hours→hour, minutes→minute, seconds→second
- **类型**：i8→u8, i16→u8
- **验证方法**：原有的 `isValid()` 方法需要适配

## 优势

### 1. **类型统一** 🎯
- 整个项目使用相同的时间日期类型
- 减少类型转换和数据不一致问题
- 提高代码复用性

### 2. **内存优化** 💾
- wibot::Date 使用 u8 字段，内存占用更小
- 减少了重复的结构定义

### 3. **维护性提升** 🔧
- 时间日期相关的改进可以统一应用
- 减少了代码重复
- 标准化的接口便于团队协作

## 迁移注意事项

### 1. 现有代码更新
```cpp
// 旧代码
time.hours = 12;
time.minutes = 30;
date.year = 2024;

// 新代码
time.hour = 12;   // 注意字段名变化
time.minute = 30;
date.year = 24;   // 注意年份需要减去2000
```

### 2. 验证逻辑调整
```cpp
// 可能需要适配 wibot::Time 和 wibot::Date 的验证方法
// 或者使用它们提供的标准方法
```

### 3. 序列化/反序列化
如果有持久化需求，需要考虑数据格式的兼容性。

## 后续建议

1. **微秒支持**：考虑是否需要扩展 wibot::Time 以支持更高精度
2. **年份处理**：评估是否需要更好的年份表示策略
3. **验证工具**：为新的时间日期格式提供验证工具
4. **文档更新**：更新相关API文档和用户指南

这次整合统一了项目中的时间日期处理，虽然在某些精度和范围上有所限制，但总体上提高了代码的一致性和维护性。