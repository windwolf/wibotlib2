#pragma once

#include "type.hpp"
#include "../base/chrono.hpp"
#include <string_view>
#include <array>
#include <optional>
#include <variant>
#include <memory>
#include <cmath>
#include <cctype>

namespace wibot::protocol {

// 使用constexpr替代宏定义
static constexpr u32 kMaxSentenceEntries = 16;
static constexpr u32 kMaxSentenceLength  = 80;

// 使用enum class提高类型安全
enum class NmeaSentenceId : u8 {
    Invalid = 0,
    Unknown,
    RMC,
    GGA,
    GSA,
    GLL,
    GST,
    GSV,
    VTG,
    ZDA,
};

enum class NmeaTalkerId : u8 {
    GN = 0,
    GP,
    BD,
};

// FAA模式使用enum class
enum class NmeaFaaMode : char {
    Autonomous   = 'A',
    Differential = 'D',
    Estimated    = 'E',
    Manual       = 'M',
    Simulated    = 'S',
    NotValid     = 'N',
    Precise      = 'P',
};

// GLL状态枚举
enum class NmeaGllStatus : char {
    DataValid    = 'A',
    DataNotValid = 'V',
};

// GSA模式和定位类型
enum class NmeaGsaMode : char {
    Auto   = 'A',
    Forced = 'M',
};

enum class NmeaGsaFixType : u8 {
    None  = 1,
    Fix2D = 2,
    Fix3D = 3,
};

// 现代化的强类型数据结构
struct NmeaSentence {
    NmeaTalkerId   talker;
    NmeaSentenceId sentenceId;
};

// 更安全的浮点数表示
struct NmeaFloat {
    i32 value{0};
    i32 scale{0};

    constexpr NmeaFloat() = default;
    constexpr NmeaFloat(i32 v, i32 s) : value(v), scale(s) {
    }

    [[nodiscard]] constexpr f32 toFloat() const noexcept {
        return (scale == 0) ? NAN : static_cast<f32>(value) / static_cast<f32>(scale);
    }

    [[nodiscard]] constexpr f32 toCoordinate() const noexcept {
        if (scale == 0) return NAN;
        const i32 degrees = value / (scale * 100);
        const i32 minutes = value % (scale * 100);
        return static_cast<f32>(degrees) + static_cast<f32>(minutes) / (60.0f * scale);
    }

    [[nodiscard]] constexpr bool isValid() const noexcept {
        return scale != 0;
    }
};

// 使用 wibot::Time 和 wibot::Date 来代替自定义的时间日期结构
using NmeaTime = wibot::Time;
using NmeaDate = wibot::Date;

// 现代化的NMEA句子数据结构，使用更好的内存布局
struct RmcData {
    NmeaTime  time;
    NmeaDate  date;
    NmeaFloat latitude;
    NmeaFloat longitude;
    NmeaFloat speed;
    NmeaFloat course;
    NmeaFloat variation;
    bool      valid{false};
};

struct GgaData {
    NmeaTime  time;
    NmeaFloat latitude;
    NmeaFloat longitude;
    NmeaFloat hdop;
    NmeaFloat altitude;
    NmeaFloat height;
    NmeaFloat dgps_age;
    i32       fix_quality{0};
    i32       satellites_tracked{0};
    char      altitude_units{'M'};
    char      height_units{'M'};
};

struct GllData {
    NmeaTime      time;
    NmeaFloat     latitude;
    NmeaFloat     longitude;
    NmeaGllStatus status{NmeaGllStatus::DataNotValid};
    NmeaFaaMode   mode{NmeaFaaMode::NotValid};
};

struct GstData {
    NmeaTime  time;
    NmeaFloat rms_deviation;
    NmeaFloat semi_major_deviation;
    NmeaFloat semi_minor_deviation;
    NmeaFloat semi_major_orientation;
    NmeaFloat latitude_error_deviation;
    NmeaFloat longitude_error_deviation;
    NmeaFloat altitude_error_deviation;
};

struct GsaData {
    std::array<i32, 12> sats{};
    NmeaFloat           pdop;
    NmeaFloat           hdop;
    NmeaFloat           vdop;
    NmeaGsaMode         mode{NmeaGsaMode::Auto};
    NmeaGsaFixType      fix_type{NmeaGsaFixType::None};
};

struct SatInfo {
    i32 nr{0};
    i32 elevation{0};
    i32 azimuth{0};
    i32 snr{0};
};

struct GsvData {
    std::array<SatInfo, 4> sats{};
    i32                    total_msgs{0};
    i32                    msg_nr{0};
    i32                    total_sats{0};
};

struct VtgData {
    NmeaFloat   true_track_degrees;
    NmeaFloat   magnetic_track_degrees;
    NmeaFloat   speed_knots;
    NmeaFloat   speed_kph;
    NmeaFaaMode faa_mode{NmeaFaaMode::NotValid};
};

struct ZdaData {
    NmeaTime time;
    NmeaDate date;
    i32      hour_offset{0};
    i32      minute_offset{0};
};

// 现代化的句子解析器接口
class SentenceParserInterface {
   public:
    virtual ~SentenceParserInterface() = default;
    [[nodiscard]] virtual Result parse(std::string_view sentence, void* data) const noexcept = 0;
    [[nodiscard]] virtual NmeaSentenceId getSentenceId() const noexcept                      = 0;
    [[nodiscard]] virtual bool           matches(std::string_view sentence) const noexcept   = 0;
};

// 简化的解析器基类 - 移除不必要的CRTP
class SentenceParserBase : public SentenceParserInterface {
   public:
    explicit SentenceParserBase(NmeaSentenceId id, std::string_view type_str) noexcept
        : sentence_id_(id), type_str_(type_str) {
    }

    [[nodiscard]] NmeaSentenceId getSentenceId() const noexcept final {
        return sentence_id_;
    }

    [[nodiscard]] bool matches(std::string_view sentence) const noexcept final {
        return sentence.length() >= 5 && sentence.substr(2, 3) == type_str_;
    }

   protected:
    NmeaSentenceId   sentence_id_;
    std::string_view type_str_;
};

// 具体的解析器实现
class RmcParser : public SentenceParserBase {
   public:
    RmcParser() : SentenceParserBase(NmeaSentenceId::RMC, "RMC") {
    }
    [[nodiscard]] Result parse(std::string_view sentence, void* data) const noexcept override;
};

class GgaParser : public SentenceParserBase {
   public:
    GgaParser() : SentenceParserBase(NmeaSentenceId::GGA, "GGA") {
    }
    [[nodiscard]] Result parse(std::string_view sentence, void* data) const noexcept override;
};

class GsaParser : public SentenceParserBase {
   public:
    GsaParser() : SentenceParserBase(NmeaSentenceId::GSA, "GSA") {
    }
    [[nodiscard]] Result parse(std::string_view sentence, void* data) const noexcept override;
};

class GllParser : public SentenceParserBase {
   public:
    GllParser() : SentenceParserBase(NmeaSentenceId::GLL, "GLL") {
    }
    [[nodiscard]] Result parse(std::string_view sentence, void* data) const noexcept override;
};

class GstParser : public SentenceParserBase {
   public:
    GstParser() : SentenceParserBase(NmeaSentenceId::GST, "GST") {
    }
    [[nodiscard]] Result parse(std::string_view sentence, void* data) const noexcept override;
};

class GsvParser : public SentenceParserBase {
   public:
    GsvParser() : SentenceParserBase(NmeaSentenceId::GSV, "GSV") {
    }
    [[nodiscard]] Result parse(std::string_view sentence, void* data) const noexcept override;
};

class VtgParser : public SentenceParserBase {
   public:
    VtgParser() : SentenceParserBase(NmeaSentenceId::VTG, "VTG") {
    }
    [[nodiscard]] Result parse(std::string_view sentence, void* data) const noexcept override;
};

class ZdaParser : public SentenceParserBase {
   public:
    ZdaParser() : SentenceParserBase(NmeaSentenceId::ZDA, "ZDA") {
    }
    [[nodiscard]] Result parse(std::string_view sentence, void* data) const noexcept override;
};

// 现代化的NMEA解析器，使用constexpr查找表和更高效的算法
class NmeaParser {
   public:
    NmeaParser();
    ~NmeaParser() = default;

    // 禁用拷贝，允许移动
    NmeaParser(const NmeaParser&)            = delete;
    NmeaParser& operator=(const NmeaParser&) = delete;
    NmeaParser(NmeaParser&&)                 = default;
    NmeaParser& operator=(NmeaParser&&)      = default;

    [[nodiscard]] Result parse(std::string_view sentence, NmeaSentenceId* id, void* data) noexcept;

    // 检查句子有效性
    [[nodiscard]] static bool isValidSentence(std::string_view sentence,
                                              bool             strict = true) noexcept;

    // 计算校验和
    [[nodiscard]] static u8 calculateChecksum(std::string_view sentence) noexcept;

    // 提取talker ID
    [[nodiscard]] static std::optional<NmeaTalkerId> getTalkerId(
        std::string_view sentence) noexcept;

   private:
    std::array<std::unique_ptr<SentenceParserInterface>,
               static_cast<size_t>(NmeaSentenceId::ZDA) + 1>
        parsers_;

    [[nodiscard]] const SentenceParserInterface* findParser(
        std::string_view sentence) const noexcept;
};

// 工具类用于高效字符串解析
class SentenceUtils {
   public:
    // 高效的字段分割器
    class FieldIterator {
       public:
        explicit FieldIterator(std::string_view sentence) noexcept;

        [[nodiscard]] bool hasNext() const noexcept {
            return current_pos_ < sentence_.length();
        }
        [[nodiscard]] std::string_view next() noexcept;
        [[nodiscard]] std::string_view peek() const noexcept;

       private:
        std::string_view sentence_;
        size_t           current_pos_;
        size_t           field_start_;
    };

    // 使用constexpr查找表的十六进制转换
    [[nodiscard]] static constexpr i32 hexToInt(char c) noexcept {
        if (c >= '0' && c <= '9') return c - '0';
        if (c >= 'A' && c <= 'F') return c - 'A' + 10;
        if (c >= 'a' && c <= 'f') return c - 'a' + 10;
        return -1;
    }

    // 高效字段检查
    [[nodiscard]] static constexpr bool isFieldChar(char c) noexcept {
        return (c >= 32 && c <= 126) && c != ',' && c != '*';
    }

    // 解析浮点数到NmeaFloat结构
    [[nodiscard]] static NmeaFloat parseFloat(std::string_view field) noexcept;

    // 解析整数
    [[nodiscard]] static i32 parseInt(std::string_view field) noexcept;

    // 解析时间
    [[nodiscard]] static NmeaTime parseTime(std::string_view field) noexcept;

    // 解析日期
    [[nodiscard]] static NmeaDate parseDate(std::string_view field) noexcept;

    // 解析方向（N/S/E/W）
    [[nodiscard]] static i32 parseDirection(char c) noexcept;

    // 解析坐标（纬度/经度）含方向
    [[nodiscard]] static NmeaFloat parseCoordinate(std::string_view coord_field,
                                                   std::string_view dir_field) noexcept;
};

// 安全地获取下一个字段的辅助器
class FieldParser {
   public:
    explicit FieldParser(SentenceUtils::FieldIterator& fields) noexcept : fields_(fields) {
    }

    [[nodiscard]] bool hasError() const noexcept {
        return has_error_;
    }

    [[nodiscard]] std::string_view nextField() noexcept {
        if (!fields_.hasNext()) {
            has_error_ = true;
            return {};
        }
        return fields_.next();
    }

    [[nodiscard]] i32 parseInt() noexcept {
        const auto field = nextField();
        return hasError() ? 0 : SentenceUtils::parseInt(field);
    }

    [[nodiscard]] NmeaFloat parseFloat() noexcept {
        const auto field = nextField();
        return hasError() ? NmeaFloat{} : SentenceUtils::parseFloat(field);
    }

    [[nodiscard]] NmeaTime parseTime() noexcept {
        const auto field = nextField();
        return hasError() ? NmeaTime{} : SentenceUtils::parseTime(field);
    }

    [[nodiscard]] NmeaDate parseDate() noexcept {
        const auto field = nextField();
        return hasError() ? NmeaDate{} : SentenceUtils::parseDate(field);
    }

    [[nodiscard]] NmeaFloat parseCoordinate() noexcept {
        const auto coord_field = nextField();
        const auto dir_field   = nextField();
        return hasError() ? NmeaFloat{} : SentenceUtils::parseCoordinate(coord_field, dir_field);
    }

    [[nodiscard]] NmeaGsaMode parseGsaMode() noexcept {
        const auto field = nextField();
        if (hasError() || field.empty()) {
            return NmeaGsaMode::Auto;
        }
        return static_cast<NmeaGsaMode>(field[0]);
    }

    [[nodiscard]] NmeaGsaFixType parseGsaFixType() noexcept {
        const auto int_value = parseInt();
        return hasError() ? NmeaGsaFixType::None : static_cast<NmeaGsaFixType>(int_value);
    }

    [[nodiscard]] NmeaGllStatus parseGllStatus() noexcept {
        const auto field = nextField();
        if (hasError() || field.empty()) {
            return NmeaGllStatus::DataNotValid;
        }
        return static_cast<NmeaGllStatus>(field[0]);
    }

    [[nodiscard]] NmeaFaaMode parseFaaMode() noexcept {
        const auto field = nextField();
        if (hasError() || field.empty()) {
            return NmeaFaaMode::NotValid;
        }
        return static_cast<NmeaFaaMode>(field[0]);
    }

   private:
    SentenceUtils::FieldIterator& fields_;
    bool                          has_error_{false};
};

}  // namespace wibot::protocol
