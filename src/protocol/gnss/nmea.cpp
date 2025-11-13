
#include "nmea.hpp"
#include <cstdlib>
#include <cstring>
#include <cctype>
#include <algorithm>

namespace wibot::protocol {

// ========================================================================================
// 现代化的工具函数，使用constexpr和更高效的算法
// ========================================================================================

// ========================================================================================
// SentenceUtils 实现
// ========================================================================================

NmeaFloat SentenceUtils::parseFloat(std::string_view field) noexcept {
    if (field.empty()) {
        return NmeaFloat{};
    }

    i32    sign  = 1;
    i32    value = 0;
    i32    scale = 0;
    size_t pos   = 0;

    // 处理符号
    if (field[0] == '+') {
        pos = 1;
    } else if (field[0] == '-') {
        sign = -1;
        pos  = 1;
    }

    // 跳过前导空格（虽然NMEA标准不允许，但一些设备可能有）
    while (pos < field.length() && field[pos] == ' ') {
        ++pos;
    }

    // 解析整数部分
    while (pos < field.length() && std::isdigit(field[pos])) {
        const i32 digit = field[pos] - '0';
        if (value > (INT32_MAX - digit) / 10) {
            // 溢出处理
            return NmeaFloat{};
        }
        value = value * 10 + digit;
        ++pos;
    }

    // 解析小数部分
    if (pos < field.length() && field[pos] == '.') {
        ++pos;
        scale = 1;
        while (pos < field.length() && std::isdigit(field[pos]) && scale <= 1000000) {
            const i32 digit = field[pos] - '0';
            if (value > (INT32_MAX - digit) / 10) {
                break;  // 防止溢出，截断精度
            }
            value = value * 10 + digit;
            scale *= 10;
            ++pos;
        }
    } else {
        scale = 1;
    }

    return NmeaFloat{value * sign, scale};
}

i32 SentenceUtils::parseInt(std::string_view field) noexcept {
    if (field.empty()) {
        return 0;
    }

    char*      end;
    const auto result = std::strtol(field.data(), &end, 10);

    // 检查是否完全解析且在范围内
    if (end == field.data() + field.length() && result >= INT32_MIN && result <= INT32_MAX) {
        return static_cast<i32>(result);
    }
    return 0;
}

NmeaTime SentenceUtils::parseTime(std::string_view field) noexcept {
    if (field.length() < 6) {
        return NmeaTime{};
    }

    // 解析HHMMSS格式
    const auto hours_str   = field.substr(0, 2);
    const auto minutes_str = field.substr(2, 2);
    const auto seconds_str = field.substr(4, 2);

    const u8 hours   = static_cast<u8>(parseInt(hours_str));
    const u8 minutes = static_cast<u8>(parseInt(minutes_str));
    const u8 seconds = static_cast<u8>(parseInt(seconds_str));

    // 注意：base/chrono.hpp 中的 Time 结构没有 microseconds 字段
    // 如果需要微秒精度，可能需要扩展 base Time 结构或使用自定义的时间结构

    return NmeaTime{hours, minutes, seconds};
}

NmeaDate SentenceUtils::parseDate(std::string_view field) noexcept {
    if (field.length() != 6) {
        return NmeaDate{};
    }

    const u8 day   = static_cast<u8>(parseInt(field.substr(0, 2)));
    const u8 month = static_cast<u8>(parseInt(field.substr(2, 2)));
    u8       year  = static_cast<u8>(parseInt(field.substr(4, 2)));

    // base/chrono.hpp 的 Date 使用 u8 年份，需要特殊处理
    // 根据项目需求决定年份基准：
    // 如果 year >= 80，表示 1980-1999，映射到 80-99
    // 如果 year < 80，表示 2000-2079，映射到 0-79
    // 这样可以在 u8 范围内表示 1980-2079 的年份范围

    return NmeaDate{year, month, day};
}

i32 SentenceUtils::parseDirection(char c) noexcept {
    switch (c) {
        case 'N':
        case 'E':
            return 1;
        case 'S':
        case 'W':
            return -1;
        default:
            return 0;
    }
}

NmeaFloat SentenceUtils::parseCoordinate(std::string_view coord_field,
                                         std::string_view dir_field) noexcept {
    auto coordinate = parseFloat(coord_field);
    if (!coordinate.isValid() || dir_field.empty()) {
        return coordinate;
    }

    const i32 direction = parseDirection(dir_field[0]);
    coordinate.value *= direction;
    return coordinate;
}

// ========================================================================================
// FieldIterator 实现 - 高效的字段分割器
// ========================================================================================

SentenceUtils::FieldIterator::FieldIterator(std::string_view sentence) noexcept
    : sentence_(sentence), current_pos_(0), field_start_(0) {
    // 跳过开头的 '$' 符号
    if (!sentence_.empty() && sentence_[0] == '$') {
        current_pos_ = 1;
        field_start_ = 1;
    }
}

std::string_view SentenceUtils::FieldIterator::next() noexcept {
    if (!hasNext()) {
        return {};
    }

    field_start_ = current_pos_;

    // 查找下一个分隔符或句子结束
    while (current_pos_ < sentence_.length() && sentence_[current_pos_] != ',' &&
           sentence_[current_pos_] != '*') {
        ++current_pos_;
    }

    const auto field = sentence_.substr(field_start_, current_pos_ - field_start_);

    // 跳过分隔符
    if (current_pos_ < sentence_.length()) {
        ++current_pos_;
    }

    return field;
}

std::string_view SentenceUtils::FieldIterator::peek() const noexcept {
    if (!hasNext()) {
        return {};
    }

    auto temp_pos = current_pos_;

    // 查找下一个分隔符或句子结束
    while (temp_pos < sentence_.length() && sentence_[temp_pos] != ',' &&
           sentence_[temp_pos] != '*') {
        ++temp_pos;
    }

    return sentence_.substr(current_pos_, temp_pos - current_pos_);
}

// ========================================================================================
// NmeaParser 实现
// ========================================================================================

NmeaParser::NmeaParser() {
    // 初始化所有解析器
    parsers_[static_cast<size_t>(NmeaSentenceId::RMC)] = std::make_unique<RmcParser>();
    parsers_[static_cast<size_t>(NmeaSentenceId::GGA)] = std::make_unique<GgaParser>();
    parsers_[static_cast<size_t>(NmeaSentenceId::GSA)] = std::make_unique<GsaParser>();
    parsers_[static_cast<size_t>(NmeaSentenceId::GLL)] = std::make_unique<GllParser>();
    parsers_[static_cast<size_t>(NmeaSentenceId::GST)] = std::make_unique<GstParser>();
    parsers_[static_cast<size_t>(NmeaSentenceId::GSV)] = std::make_unique<GsvParser>();
    parsers_[static_cast<size_t>(NmeaSentenceId::VTG)] = std::make_unique<VtgParser>();
    parsers_[static_cast<size_t>(NmeaSentenceId::ZDA)] = std::make_unique<ZdaParser>();
}

Result NmeaParser::parse(std::string_view sentence, NmeaSentenceId* id, void* data) noexcept {
    if (!isValidSentence(sentence)) {
        if (id) *id = NmeaSentenceId::Invalid;
        return Result::kError;
    }

    const auto* parser = findParser(sentence);
    if (!parser) {
        if (id) *id = NmeaSentenceId::Unknown;
        return Result::kError;
    }

    if (id) *id = parser->getSentenceId();
    return parser->parse(sentence, data);
}

bool NmeaParser::isValidSentence(std::string_view sentence, bool strict) noexcept {
    if (sentence.empty() || sentence.length() > kMaxSentenceLength + 3) {
        return false;
    }

    // 检查开头的 '$'
    if (sentence[0] != '$') {
        return false;
    }

    // 计算校验和
    u8     calculated_checksum = 0;
    size_t pos                 = 1;  // 跳过 '$'

    // 计算到 '*' 为止的校验和
    while (pos < sentence.length() && sentence[pos] != '*') {
        if (!std::isprint(sentence[pos])) {
            return false;
        }
        calculated_checksum ^= static_cast<u8>(sentence[pos]);
        ++pos;
    }

    // 检查校验和
    if (pos < sentence.length() && sentence[pos] == '*') {
        if (pos + 2 >= sentence.length()) {
            return false;  // 校验和不完整
        }

        const i32 upper = SentenceUtils::hexToInt(sentence[pos + 1]);
        const i32 lower = SentenceUtils::hexToInt(sentence[pos + 2]);

        if (upper == -1 || lower == -1) {
            return false;
        }

        const u8 expected_checksum = static_cast<u8>((upper << 4) | lower);
        if (calculated_checksum != expected_checksum) {
            return false;
        }

        pos += 3;  // 跳过 '*XX'
    } else if (strict) {
        return false;  // 严格模式下必须有校验和
    }

    // 检查句子结尾
    while (pos < sentence.length()) {
        if (sentence[pos] != '\r' && sentence[pos] != '\n') {
            return false;
        }
        ++pos;
    }

    return true;
}

u8 NmeaParser::calculateChecksum(std::string_view sentence) noexcept {
    u8     checksum  = 0;
    size_t start_pos = (sentence.empty() || sentence[0] != '$') ? 0 : 1;

    for (size_t i = start_pos; i < sentence.length() && sentence[i] != '*'; ++i) {
        checksum ^= static_cast<u8>(sentence[i]);
    }

    return checksum;
}

std::optional<NmeaTalkerId> NmeaParser::getTalkerId(std::string_view sentence) noexcept {
    if (sentence.length() < 3) {
        return std::nullopt;
    }

    const auto talker_str = sentence.substr(1, 2);  // 跳过 '$'

    if (talker_str == "GN") return NmeaTalkerId::GN;
    if (talker_str == "GP") return NmeaTalkerId::GP;
    if (talker_str == "BD") return NmeaTalkerId::BD;

    return std::nullopt;
}

const SentenceParserInterface* NmeaParser::findParser(std::string_view sentence) const noexcept {
    // 使用线性搜索查找匹配的解析器
    // 对于少量解析器，这比哈希表更高效
    for (const auto& parser : parsers_) {
        if (parser && parser->matches(sentence)) {
            return parser.get();
        }
    }
    return nullptr;
}

// ========================================================================================
// 具体解析器实现 - 现代化的NMEA句子解析
// ========================================================================================

Result RmcParser::parse(std::string_view sentence, void* data) const noexcept {
    auto* rmc_data = static_cast<RmcData*>(data);
    if (!rmc_data) {
        return Result::kInvalidParameter;
    }

    SentenceUtils::FieldIterator fields(sentence);
    FieldParser                  parser(fields);

    // 跳过句子类型字段 (GPRMC)
    [[maybe_unused]] auto sentence_type = parser.nextField();

    // 解析时间
    rmc_data->time = parser.parseTime();

    // 解析状态
    const auto status_field = parser.nextField();
    rmc_data->valid         = (!status_field.empty() && status_field[0] == 'A');

    // 解析纬度含方向
    rmc_data->latitude = parser.parseCoordinate();

    // 解析经度含方向
    rmc_data->longitude = parser.parseCoordinate();

    // 解析速度
    rmc_data->speed = parser.parseFloat();

    // 解析航向
    rmc_data->course = parser.parseFloat();

    // 解析日期
    rmc_data->date = parser.parseDate();

    // 解析磁偏角
    rmc_data->variation = parser.parseFloat();

    // 解析磁偏角方向
    if (fields.hasNext()) {
        const auto var_dir = fields.next();
        if (!var_dir.empty()) {
            rmc_data->variation.value *= SentenceUtils::parseDirection(var_dir[0]);
        }
    }

    return parser.hasError() ? Result::kError : Result::kOk;
}

Result GgaParser::parse(std::string_view sentence, void* data) const noexcept {
    auto* gga_data = static_cast<GgaData*>(data);
    if (!gga_data) {
        return Result::kInvalidParameter;
    }

    SentenceUtils::FieldIterator fields(sentence);
    FieldParser                  parser(fields);

    // 跳过句子类型字段 (GPGGA)
    [[maybe_unused]] auto sentence_type = parser.nextField();

    // 解析时间
    gga_data->time = parser.parseTime();

    // 解析纬度含方向
    gga_data->latitude = parser.parseCoordinate();

    // 解析经度含方向
    gga_data->longitude = parser.parseCoordinate();

    // 解析定位质量
    gga_data->fix_quality = parser.parseInt();

    // 解析卫星数量
    gga_data->satellites_tracked = parser.parseInt();

    // 解析HDOP
    gga_data->hdop = parser.parseFloat();

    // 解析海拔
    gga_data->altitude = parser.parseFloat();

    // 解析海拔单位
    const auto alt_units     = parser.nextField();
    gga_data->altitude_units = alt_units.empty() ? 'M' : alt_units[0];

    // 解析大地水准面高度
    gga_data->height = parser.parseFloat();

    // 解析高度单位
    const auto height_units = parser.nextField();
    gga_data->height_units  = height_units.empty() ? 'M' : height_units[0];

    // 解析DGPS年龄（可选）
    if (fields.hasNext()) {
        gga_data->dgps_age = SentenceUtils::parseFloat(fields.next());
    }

    return parser.hasError() ? Result::kError : Result::kOk;
}

Result GsaParser::parse(std::string_view sentence, void* data) const noexcept {
    auto* gsa_data = static_cast<GsaData*>(data);
    if (!gsa_data) {
        return Result::kInvalidParameter;
    }

    SentenceUtils::FieldIterator fields(sentence);
    FieldParser                  parser(fields);

    // 跳过句子类型字段
    [[maybe_unused]] auto sentence_type = parser.nextField();

    // 解析模式
    gsa_data->mode = parser.parseGsaMode();

    // 解析定位类型
    gsa_data->fix_type = parser.parseGsaFixType();

    // 解析卫星ID（12个）- 使用循环减少重复
    for (size_t i = 0; i < gsa_data->sats.size(); ++i) {
        gsa_data->sats[i] = parser.parseInt();
    }

    // 解析DOP值
    gsa_data->pdop = parser.parseFloat();
    gsa_data->hdop = parser.parseFloat();
    gsa_data->vdop = parser.parseFloat();

    if (parser.hasError()) {
        return Result::kError;
    }

    return Result::kOk;
}

Result GllParser::parse(std::string_view sentence, void* data) const noexcept {
    auto* gll_data = static_cast<GllData*>(data);
    if (!gll_data) {
        return Result::kInvalidParameter;
    }

    SentenceUtils::FieldIterator fields(sentence);
    FieldParser                  parser(fields);

    // 跳过句子类型字段
    [[maybe_unused]] auto sentence_type = parser.nextField();

    // 解析纬度含方向
    gll_data->latitude = parser.parseCoordinate();

    // 解析经度含方向
    gll_data->longitude = parser.parseCoordinate();

    // 解析时间
    gll_data->time = parser.parseTime();

    // 解析状态
    gll_data->status = parser.parseGllStatus();

    // 解析FAA模式（可选）
    gll_data->mode = parser.parseFaaMode();

    if (parser.hasError()) {
        return Result::kError;
    }

    return Result::kOk;
}

Result GstParser::parse(std::string_view sentence, void* data) const noexcept {
    auto* gst_data = static_cast<GstData*>(data);
    if (!gst_data) {
        return Result::kInvalidParameter;
    }

    SentenceUtils::FieldIterator fields(sentence);
    FieldParser                  parser(fields);

    // 跳过句子类型字段
    [[maybe_unused]] auto sentence_type = parser.nextField();

    // 解析时间
    gst_data->time = parser.parseTime();

    // 解析RMS偏差
    gst_data->rms_deviation = parser.parseFloat();

    // 解析半长轴偏差
    gst_data->semi_major_deviation = parser.parseFloat();

    // 解析半短轴偏差
    gst_data->semi_minor_deviation = parser.parseFloat();

    // 解析半长轴方向
    gst_data->semi_major_orientation = parser.parseFloat();

    // 解析纬度误差偏差
    gst_data->latitude_error_deviation = parser.parseFloat();

    // 解析经度误差偏差
    gst_data->longitude_error_deviation = parser.parseFloat();

    // 解析高度误差偏差
    gst_data->altitude_error_deviation = parser.parseFloat();

    if (parser.hasError()) {
        return Result::kError;
    }

    return Result::kOk;
}

Result GsvParser::parse(std::string_view sentence, void* data) const noexcept {
    auto* gsv_data = static_cast<GsvData*>(data);
    if (!gsv_data) {
        return Result::kInvalidParameter;
    }

    SentenceUtils::FieldIterator fields(sentence);
    FieldParser                  parser(fields);

    // 跳过句子类型字段
    [[maybe_unused]] auto sentence_type = parser.nextField();

    // 解析总消息数
    gsv_data->total_msgs = parser.parseInt();

    // 解析当前消息编号
    gsv_data->msg_nr = parser.parseInt();

    // 解析总卫星数
    gsv_data->total_sats = parser.parseInt();

    // 解析卫星信息（最多4颗）
    for (size_t i = 0; i < gsv_data->sats.size(); ++i) {
        auto& sat = gsv_data->sats[i];

        // 卫星编号
        sat.nr = parser.parseInt();
        if (parser.hasError()) break;

        // 仰角
        sat.elevation = parser.parseInt();
        if (parser.hasError()) break;

        // 方位角
        sat.azimuth = parser.parseInt();
        if (parser.hasError()) break;

        // 信噪比
        sat.snr = parser.parseInt();
        if (parser.hasError()) break;
    }

    return Result::kOk;
}

Result VtgParser::parse(std::string_view sentence, void* data) const noexcept {
    auto* vtg_data = static_cast<VtgData*>(data);
    if (!vtg_data) {
        return Result::kInvalidParameter;
    }

    SentenceUtils::FieldIterator fields(sentence);
    FieldParser                  parser(fields);

    // 跳过句子类型字段
    [[maybe_unused]] auto sentence_type = parser.nextField();

    // 解析真航向
    vtg_data->true_track_degrees = parser.parseFloat();

    // 跳过 'T' 指示符
    [[maybe_unused]] auto t_indicator = parser.nextField();

    // 解析磁航向
    vtg_data->magnetic_track_degrees = parser.parseFloat();

    // 跳过 'M' 指示符
    [[maybe_unused]] auto m_indicator = parser.nextField();

    // 解析节速度
    vtg_data->speed_knots = parser.parseFloat();

    // 跳过 'N' 指示符
    [[maybe_unused]] auto n_indicator = parser.nextField();

    // 解析公里/小时速度
    vtg_data->speed_kph = parser.parseFloat();

    // 跳过 'K' 指示符
    [[maybe_unused]] auto k_indicator = parser.nextField();

    // 解析FAA模式（可选）
    vtg_data->faa_mode = parser.parseFaaMode();

    if (parser.hasError()) {
        return Result::kError;
    }

    return Result::kOk;
}

Result ZdaParser::parse(std::string_view sentence, void* data) const noexcept {
    auto* zda_data = static_cast<ZdaData*>(data);
    if (!zda_data) {
        return Result::kInvalidParameter;
    }

    SentenceUtils::FieldIterator fields(sentence);
    FieldParser                  parser(fields);

    // 跳过句子类型字段
    [[maybe_unused]] auto sentence_type = parser.nextField();

    // 解析时间
    zda_data->time = parser.parseTime();

    // 解析日
    const auto day = parser.parseInt();

    // 解析月
    const auto month = parser.parseInt();

    // 解析年
    const auto full_year = parser.parseInt();
    // 将4位年份转换为适合 u8 的格式（例如，相对于2000年的偏移）
    const u8   year      = static_cast<u8>(full_year >= 2000 ? full_year - 2000 : 0);

    zda_data->date = NmeaDate{static_cast<u8>(year), static_cast<u8>(month), static_cast<u8>(day)};

    // 解析时区小时偏移
    zda_data->hour_offset = parser.parseInt();

    // 解析时区分钟偏移
    zda_data->minute_offset = parser.parseInt();

    if (parser.hasError()) {
        return Result::kError;
    }

    // 验证时区偏移的合理性
    if (std::abs(zda_data->hour_offset) > 13 || zda_data->minute_offset > 59 ||
        zda_data->minute_offset < 0) {
        return Result::kError;
    }

    return Result::kOk;
}

}  // namespace wibot::protocol
