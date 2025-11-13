# NMEA枚举命名规范更新总结

## 更新概述

为了提高代码的命名空间清晰度和避免潜在的命名冲突，对NMEA协议解析器中的所有枚举类型添加了"Nmea"前缀。

## 枚举类型更新对比

### 1. 句子ID枚举
```cpp
// 更新前
enum class SentenceId : u8 {
    Invalid = 0,
    Unknown,
    RMC,
    GGA,
    // ...
};

// 更新后
enum class NmeaSentenceId : u8 {
    Invalid = 0,
    Unknown,
    RMC,
    GGA,
    // ...
};
```

### 2. Talker ID枚举
```cpp
// 更新前
enum class TalkerId : u8 {
    GN = 0,
    GP,
    BD,
};

// 更新后
enum class NmeaTalkerId : u8 {
    GN = 0,
    GP,
    BD,
};
```

### 3. FAA模式枚举
```cpp
// 更新前
enum class FaaMode : char {
    Autonomous   = 'A',
    Differential = 'D',
    // ...
};

// 更新后
enum class NmeaFaaMode : char {
    Autonomous   = 'A',
    Differential = 'D',
    // ...
};
```

### 4. GLL状态枚举
```cpp
// 更新前
enum class GllStatus : char {
    DataValid    = 'A',
    DataNotValid = 'V',
};

// 更新后
enum class NmeaGllStatus : char {
    DataValid    = 'A',
    DataNotValid = 'V',
};
```

### 5. GSA模式和定位类型枚举
```cpp
// 更新前
enum class GsaMode : char {
    Auto   = 'A',
    Forced = 'M',
};

enum class GsaFixType : u8 {
    None  = 1,
    Fix2D = 2,
    Fix3D = 3,
};

// 更新后
enum class NmeaGsaMode : char {
    Auto   = 'A',
    Forced = 'M',
};

enum class NmeaGsaFixType : u8 {
    None  = 1,
    Fix2D = 2,
    Fix3D = 3,
};
```

## 数据结构更新

### 主句子结构
```cpp
// 更新前
struct Sentence {
    TalkerId   talker;
    SentenceId sentenceId;
};

// 更新后
struct NmeaSentence {
    NmeaTalkerId   talker;
    NmeaSentenceId sentenceId;
};
```

### 各种数据结构中的枚举引用
```cpp
// GLL数据结构
struct GllData {
    Time           time;
    Float          latitude;
    Float          longitude;
    NmeaGllStatus  status{NmeaGllStatus::DataNotValid};  // 更新
    NmeaFaaMode    mode{NmeaFaaMode::NotValid};          // 更新
};

// GSA数据结构
struct GsaData {
    std::array<i32, 12> sats{};
    Float               pdop;
    Float               hdop;
    Float               vdop;
    NmeaGsaMode         mode{NmeaGsaMode::Auto};         // 更新
    NmeaGsaFixType      fix_type{NmeaGsaFixType::None};  // 更新
};

// VTG数据结构
struct VtgData {
    Float       true_track_degrees;
    Float       magnetic_track_degrees;
    Float       speed_knots;
    Float       speed_kph;
    NmeaFaaMode faa_mode{NmeaFaaMode::NotValid};         // 更新
};
```

## 接口更新

### 解析器接口
```cpp
// 更新前
class SentenceParserInterface {
public:
    virtual ~SentenceParserInterface() = default;
    [[nodiscard]] virtual SentenceId getSentenceId() const noexcept = 0;
    // ...
};

// 更新后
class SentenceParserInterface {
public:
    virtual ~SentenceParserInterface() = default;
    [[nodiscard]] virtual NmeaSentenceId getSentenceId() const noexcept = 0;
    // ...
};
```

### 主解析器类
```cpp
// 更新前
class NmeaParser {
public:
    [[nodiscard]] Result parse(std::string_view sentence, SentenceId* id, void* data) noexcept;
    [[nodiscard]] static std::optional<TalkerId> getTalkerId(std::string_view sentence) noexcept;
    // ...
};

// 更新后
class NmeaParser {
public:
    [[nodiscard]] Result parse(std::string_view sentence, NmeaSentenceId* id, void* data) noexcept;
    [[nodiscard]] static std::optional<NmeaTalkerId> getTalkerId(std::string_view sentence) noexcept;
    // ...
};
```

## 具体解析器更新

### 构造函数更新
```cpp
// 示例：RMC解析器
// 更新前
RmcParser() : SentenceParserBase(SentenceId::RMC, "RMC") {}

// 更新后
RmcParser() : SentenceParserBase(NmeaSentenceId::RMC, "RMC") {}
```

## 实现文件更新

### 解析器初始化
```cpp
// 更新前
parsers_[static_cast<size_t>(SentenceId::RMC)] = std::make_unique<RmcParser>();

// 更新后
parsers_[static_cast<size_t>(NmeaSentenceId::RMC)] = std::make_unique<RmcParser>();
```

### 枚举值使用
```cpp
// 更新前
gsa_data->mode = mode_field.empty() ? GsaMode::Auto : static_cast<GsaMode>(mode_field[0]);

// 更新后
gsa_data->mode = mode_field.empty() ? NmeaGsaMode::Auto : static_cast<NmeaGsaMode>(mode_field[0]);
```

## 示例代码更新

### 使用示例
```cpp
// 更新前
auto talker_id = NmeaParser::getTalkerId(sentence);
if (talker_id) {
    switch (*talker_id) {
        case TalkerId::GP: std::cout << "GPS"; break;
        case TalkerId::GN: std::cout << "GNSS"; break;
        // ...
    }
}

// 更新后
auto talker_id = NmeaParser::getTalkerId(sentence);
if (talker_id) {
    switch (*talker_id) {
        case NmeaTalkerId::GP: std::cout << "GPS"; break;
        case NmeaTalkerId::GN: std::cout << "GNSS"; break;
        // ...
    }
}
```

## 更新优势

### 1. **命名空间清晰** 🏷️
- 所有NMEA相关枚举都有明确的"Nmea"前缀
- 避免与其他协议或库的枚举类型冲突
- 提高代码的可读性和自文档化

### 2. **类型安全** 🛡️
- 保持了enum class的所有优势
- 编译时类型检查
- 防止意外的类型转换

### 3. **一致性** 📐
- 整个NMEA模块使用统一的命名规范
- 符合C++最佳实践
- 便于代码维护和扩展

### 4. **向后兼容性** 🔄
- 虽然是破坏性更改，但更改是系统性的
- 编译器会帮助发现所有需要更新的地方
- 更新后的代码更加健壮

## 迁移指南

如果有现有代码使用了旧的枚举名称，需要进行以下更新：

1. **替换枚举类型名称**：
   - `SentenceId` → `NmeaSentenceId`
   - `TalkerId` → `NmeaTalkerId`
   - `FaaMode` → `NmeaFaaMode`
   - `GllStatus` → `NmeaGllStatus`
   - `GsaMode` → `NmeaGsaMode`
   - `GsaFixType` → `NmeaGsaFixType`

2. **更新变量声明**：
   ```cpp
   NmeaSentenceId sentence_id;  // 替代 SentenceId sentence_id;
   ```

3. **更新switch语句**：
   ```cpp
   switch (sentence_id) {
       case NmeaSentenceId::RMC:  // 替代 SentenceId::RMC
           // ...
           break;
   }
   ```

4. **更新函数签名**：
   ```cpp
   Result parse(std::string_view sentence, NmeaSentenceId* id, void* data);
   ```

这个更新使得NMEA解析器的API更加专业和规范，为将来可能的扩展和维护奠定了更好的基础。