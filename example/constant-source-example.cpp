#include "constant-source.hpp"
#include <iostream>

namespace wibot {

void constant_source_example() {
  // 创建一个4通道的i16类型常量源，初始值为100
  ConstantSource<i16, 4> constantSrc(100);

  std::cout << "=== ConstantSource<i16, 4> 使用示例 ===" << std::endl;

  // 显示初始值
  std::cout << "初始值（所有通道为100）:" << std::endl;
  for (u8 ch = 0; ch < 4; ch++) {
    std::cout << "  通道 " << (int)ch << ": " << constantSrc.getValue(ch)
              << std::endl;
  }

  // 设置单个通道的值
  constantSrc.setValue(0, 200);
  constantSrc.setValue(1, 300);
  constantSrc.setValue(2, 400);
  constantSrc.setValue(3, 500);

  std::cout << "\n设置各通道不同值后:" << std::endl;
  for (u8 ch = 0; ch < 4; ch++) {
    std::cout << "  通道 " << (int)ch << ": " << constantSrc.getValue(ch)
              << std::endl;
  }

  // 设置所有通道为相同值
  constantSrc.setAllValues(1000);

  std::cout << "\n设置所有通道为1000后:" << std::endl;
  for (u8 ch = 0; ch < 4; ch++) {
    std::cout << "  通道 " << (int)ch << ": " << constantSrc.getValue(ch)
              << std::endl;
  }

  // 使用数组设置多个值
  i16 newValues[4] = {10, 20, 30, 40};
  constantSrc.setValues(newValues);

  std::cout << "\n使用数组设置多个值后:" << std::endl;
  for (u8 ch = 0; ch < 4; ch++) {
    std::cout << "  通道 " << (int)ch << ": " << constantSrc.getValue(ch)
              << std::endl;
  }

  // 获取所有值的指针
  i16 *allValues = constantSrc.getValues();
  std::cout << "\n通过getValues()获取所有值:" << std::endl;
  for (u8 ch = 0; ch < 4; ch++) {
    std::cout << "  通道 " << (int)ch << ": " << allValues[ch] << std::endl;
  }

  // 重置
  constantSrc.reset();
  std::cout << "\n重置后（所有通道为0）:" << std::endl;
  for (u8 ch = 0; ch < 4; ch++) {
    std::cout << "  通道 " << (int)ch << ": " << constantSrc.getValue(ch)
              << std::endl;
  }

  // 测试边界情况
  std::cout << "\n测试边界情况:" << std::endl;
  std::cout << "访问无效通道(索引5): " << constantSrc.getValue(5) << std::endl;

  constantSrc.setValue(10, 999); // 设置无效通道，应该被忽略
  std::cout << "设置无效通道后，通道0的值: " << constantSrc.getValue(0)
            << std::endl;

  // 展示不同数据类型的使用
  std::cout << "\n=== 不同数据类型示例 ===" << std::endl;

  // 浮点数类型
  ConstantSource<f32, 2> floatSrc(3.14f);
  floatSrc.setValue(0, 1.23f);
  floatSrc.setValue(1, 4.56f);
  std::cout << "浮点数源 - 通道0: " << floatSrc.getValue(0)
            << ", 通道1: " << floatSrc.getValue(1) << std::endl;

  // 32位整数类型
  ConstantSource<i32, 3> intSrc(1000000);
  intSrc.setValue(0, -500000);
  intSrc.setValue(1, 0);
  intSrc.setValue(2, 999999);
  std::cout << "32位整数源:" << std::endl;
  for (u8 ch = 0; ch < 3; ch++) {
    std::cout << "  通道 " << (int)ch << ": " << intSrc.getValue(ch)
              << std::endl;
  }
}

} // namespace wibot