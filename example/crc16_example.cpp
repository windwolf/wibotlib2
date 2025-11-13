//
// CRC16 使用示例
// Created by AI Assistant on 2025/7/11.
//

#include "crc16.hpp"
#include <cstdio>

using namespace wibot;

void crc16_example() {
    // 创建一个 CRC16-MODBUS 验证器
    Crc16Validator crc16_modbus(Crc16Validator::CRC16_MODBUS, 0xFFFF, 0x0000, true, true);

    // 测试数据
    uint8_t  test_data[] = {0x01, 0x02, 0x03, 0x04, 0x05};
    uint32_t data_length = sizeof(test_data);

    // 计算 CRC16
    crc16_modbus.reset();
    crc16_modbus.calculate(test_data, data_length);
    uint16_t crc_result = crc16_modbus.get();

    printf("CRC16-MODBUS result: 0x%04X\n", crc_result);

    // 验证数据（包含CRC的完整数据包）
    uint8_t complete_data[] = {
        0x01, 0x02, 0x03, 0x04, 0x05, (uint8_t)(crc_result >> 8), (uint8_t)(crc_result & 0xFF)};

    // 重新计算并验证
    crc16_modbus.reset();
    crc16_modbus.calculate(complete_data, sizeof(complete_data) - 2);

    uint8_t received_crc[2] = {complete_data[5], complete_data[6]};
    bool    is_valid        = crc16_modbus.validate(received_crc);

    printf("Validation result: %s\n", is_valid ? "PASS" : "FAIL");
}

// 其他常用的 CRC16 配置示例
void other_crc16_examples() {
    // CRC16-CCITT
    Crc16Validator crc16_ccitt(Crc16Validator::CRC16_CCITT, 0xFFFF, 0x0000, false, false);

    // CRC16-XMODEM
    Crc16Validator crc16_xmodem(Crc16Validator::CRC16_XMODEM, 0x0000, 0x0000, false, false);

    // CRC16-USB
    Crc16Validator crc16_usb(Crc16Validator::CRC16_USB, 0xFFFF, 0xFFFF, true, true);
}
