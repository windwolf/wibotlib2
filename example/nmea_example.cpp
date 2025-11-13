/**
 * NMEA解析器使用示例
 * 展示现代化重构后的高效NMEA协议解析器的使用方法
 */

#include "wibotlib/src/protocol/gnss/nmea.hpp"
#include <iomanip>
#include <iostream>
#include <string_view>

using namespace wibot::protocol;

void printRmcData(const RmcData &data) {
  std::cout << "=== RMC Data ===" << std::endl;
  std::cout << "Valid: " << (data.valid ? "Yes" : "No") << std::endl;
  std::cout << "Time: " << std::setfill('0') << std::setw(2)
            << static_cast<int>(data.time.hours) << ":" << std::setw(2)
            << static_cast<int>(data.time.minutes) << ":" << std::setw(2)
            << static_cast<int>(data.time.seconds) << std::endl;

  if (data.latitude.isValid()) {
    std::cout << "Latitude: " << std::fixed << std::setprecision(6)
              << data.latitude.toCoordinate() << "°" << std::endl;
  }

  if (data.longitude.isValid()) {
    std::cout << "Longitude: " << std::fixed << std::setprecision(6)
              << data.longitude.toCoordinate() << "°" << std::endl;
  }

  if (data.speed.isValid()) {
    std::cout << "Speed: " << std::fixed << std::setprecision(2)
              << data.speed.toFloat() << " knots" << std::endl;
  }
}

void printGgaData(const GgaData &data) {
  std::cout << "=== GGA Data ===" << std::endl;
  std::cout << "Fix Quality: " << data.fix_quality << std::endl;
  std::cout << "Satellites: " << data.satellites_tracked << std::endl;

  if (data.altitude.isValid()) {
    std::cout << "Altitude: " << std::fixed << std::setprecision(1)
              << data.altitude.toFloat() << " " << data.altitude_units
              << std::endl;
  }

  if (data.hdop.isValid()) {
    std::cout << "HDOP: " << std::fixed << std::setprecision(1)
              << data.hdop.toFloat() << std::endl;
  }
}

int main() {
  // 创建现代化的NMEA解析器
  NmeaParser parser;

  // 测试数据 - 真实的NMEA句子
  const std::string_view test_sentences[] = {
      "$GPRMC,123519,A,4807.038,N,01131.000,E,022.4,084.4,230394,003.1,W*6A",
      "$GPGGA,123519,4807.038,N,01131.000,E,1,08,0.9,545.4,M,46.9,M,,*47",
      "$GPGSA,A,3,04,05,,09,12,,,24,,,,,2.5,1.3,2.1*39",
      "$GPGLL,4807.038,N,01131.000,E,123519,A,A*44",
      "$GPVTG,054.7,T,034.4,M,005.5,N,010.2,K*48"};

  std::cout << "现代化NMEA解析器测试" << std::endl;
  std::cout << "=====================" << std::endl << std::endl;

  for (const auto &sentence : test_sentences) {
    std::cout << "解析句子: " << sentence << std::endl;

    // 首先验证句子有效性
    if (!NmeaParser::isValidSentence(sentence)) {
      std::cout << "错误: 句子格式无效" << std::endl << std::endl;
      continue;
    }

    // 获取Talker ID
    auto talker_id = NmeaParser::getTalkerId(sentence);
    if (talker_id) {
      std::cout << "Talker ID: ";
      switch (*talker_id) {
      case NmeaTalkerId::GP:
        std::cout << "GPS";
        break;
      case NmeaTalkerId::GN:
        std::cout << "GNSS";
        break;
      case NmeaTalkerId::BD:
        std::cout << "BeiDou";
        break;
      }
      std::cout << std::endl;
    }

    // 解析句子
    NmeaSentenceId sentence_id;
    RmcData rmc_data{};
    GgaData gga_data{};

    // 根据不同的句子类型使用不同的数据结构
    void *data_ptr = nullptr;
    if (sentence.substr(3, 3) == "RMC") {
      data_ptr = &rmc_data;
    } else if (sentence.substr(3, 3) == "GGA") {
      data_ptr = &gga_data;
    }

    if (data_ptr) {
      auto result = parser.parse(sentence, &sentence_id, data_ptr);

      if (result.isOk()) {
        std::cout << "解析成功!" << std::endl;

        switch (sentence_id) {
        case NmeaSentenceId::RMC:
          printRmcData(rmc_data);
          break;
        case NmeaSentenceId::GGA:
          printGgaData(gga_data);
          break;
        default:
          std::cout << "其他类型的句子解析成功" << std::endl;
          break;
        }
      } else {
        std::cout << "解析失败: 错误码 " << result.getErrorCode() << std::endl;
      }
    } else {
      std::cout << "暂不支持此类型的句子" << std::endl;
    }

    std::cout << std::endl;
  }

  // 性能测试
  std::cout << "性能特性:" << std::endl;
  std::cout << "- 使用 string_view 避免字符串拷贝" << std::endl;
  std::cout << "- constexpr 查找表提高解析速度" << std::endl;
  std::cout << "- 现代C++类型安全" << std::endl;
  std::cout << "- 零内存分配的解析过程" << std::endl;
  std::cout << "- CRTP优化虚函数调用" << std::endl;

  return 0;
}