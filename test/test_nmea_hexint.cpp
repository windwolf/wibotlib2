#include <iostream>
#include <string_view>

// 测试 hexToInt 函数的性能和正确性
constexpr int hexToInt(char c) noexcept {
  if (c >= '0' && c <= '9')
    return c - '0';
  if (c >= 'A' && c <= 'F')
    return c - 'A' + 10;
  if (c >= 'a' && c <= 'f')
    return c - 'a' + 10;
  return -1;
}

// 测试校验和解析
bool parseChecksum(std::string_view sentence) {
  if (sentence.length() < 4)
    return false;

  // 找到 '*' 位置
  size_t pos = sentence.find('*');
  if (pos == std::string_view::npos || pos + 2 >= sentence.length()) {
    return false;
  }

  // 计算校验和
  unsigned char calculated_checksum = 0;
  for (size_t i = 1; i < pos; ++i) { // 跳过 '$'
    calculated_checksum ^= static_cast<unsigned char>(sentence[i]);
  }

  // 解析十六进制校验和
  const int upper = hexToInt(sentence[pos + 1]);
  const int lower = hexToInt(sentence[pos + 2]);

  if (upper == -1 || lower == -1) {
    return false;
  }

  const unsigned char expected_checksum =
      static_cast<unsigned char>((upper << 4) | lower);
  return calculated_checksum == expected_checksum;
}

int main() {
  // 测试用例
  std::string_view test_sentences[] = {
      "$GPRMC,123456.78,A,1234.56,N,12345.67,E,1.0,2.0,230394,003.1,W*6A",
      "$GPGGA,123456.78,1234.56,N,12345.67,E,1,08,0.9,545.4,M,46.9,M,,*47",
      "$GPGSA,A,3,01,02,03,04,05,06,07,08,09,,,12,1.0,1.0,1.0*30"};

  std::cout << "测试 hexToInt 函数替换 kHexLookup 表:\n";

  // 测试单个字符转换
  std::cout << "\n单字符测试:\n";
  char test_chars[] = {'0', '5', '9', 'A', 'F', 'a', 'f', 'G', 'z'};
  for (char c : test_chars) {
    int result = hexToInt(c);
    std::cout << "'" << c << "' -> " << result
              << (result == -1 ? " (无效)" : "") << "\n";
  }

  // 测试校验和解析
  std::cout << "\n校验和验证测试:\n";
  for (const auto &sentence : test_sentences) {
    bool valid = parseChecksum(sentence);
    std::cout << (valid ? "✓" : "✗") << " " << sentence << "\n";
  }

  std::cout << "\n所有测试完成！\n";
  return 0;
}