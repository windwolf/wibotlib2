/**
 * @file mapping-SyncPipeline-multichannel.test.cpp
 * @brief 多通道映射管道测试示例
 */

#include "../src/hal/adc-source.hpp"
#include "../src/hal/mapping-model.hpp"
#include "minunit.hpp"

namespace wibot {

/**
 * @brief 模拟ADC硬件用于测试
 */
class MockAdcHardware {
public:
  static constexpr uint8_t CHANNELS = 4;

  MockAdcHardware() {
    // 模拟4个通道的不同ADC值
    _values[0] = 1000; // 通道0: 低值
    _values[1] = 2048; // 通道1: 中值
    _values[2] = 3000; // 通道2: 高值
    _values[3] = 4095; // 通道3: 满值
  }

  uint16_t readChannel(uint8_t channel) const {
    if (channel < CHANNELS) {
      return _values[channel];
    }
    return 0;
  }

private:
  uint16_t _values[CHANNELS];
};

/**
 * @brief 测试多通道线性映射器
 */
static const char *test_multichannel_linear_mapper() {
  MockAdcHardware adcHardware;

  // 创建4通道ADC源 (12位ADC, 无偏移)
  int16_t offsets[4] = {0, 0, 0, 0};
  AdcSource<4> adcSource(adcHardware, 12, offsets);

  // 创建4通道线性映射器 - 每个通道不同的映射范围
  LinearMapper<4>::Config configs[4] = {// 通道0: 映射到温度 -40 到 125°C
                                        {.inputMin = -32768.0f,
                                         .inputMax = 32767.0f,
                                         .outputMin = -40.0f,
                                         .outputMax = 125.0f,
                                         .clampOutput = true},
                                        // 通道1: 映射到压力 0 到 10 bar
                                        {.inputMin = -32768.0f,
                                         .inputMax = 32767.0f,
                                         .outputMin = 0.0f,
                                         .outputMax = 10.0f,
                                         .clampOutput = true},
                                        // 通道2: 映射到湿度 0 到 100%
                                        {.inputMin = -32768.0f,
                                         .inputMax = 32767.0f,
                                         .outputMin = 0.0f,
                                         .outputMax = 100.0f,
                                         .clampOutput = true},
                                        // 通道3: 映射到电压 0 到 5V
                                        {.inputMin = -32768.0f,
                                         .inputMax = 32767.0f,
                                         .outputMin = 0.0f,
                                         .outputMax = 5.0f,
                                         .clampOutput = true}};

  LinearMapper<4> mapper(adcSource, configs);

  // 测试每个通道的映射结果
  float temp = mapper.getValue(0);     // 温度
  float pressure = mapper.getValue(1); // 压力
  float humidity = mapper.getValue(2); // 湿度
  float voltage = mapper.getValue(3);  // 电压

  // 验证映射结果在合理范围内
  mu_assert("温度应在范围内", temp >= -40.0f && temp <= 125.0f);
  mu_assert("压力应在范围内", pressure >= 0.0f && pressure <= 10.0f);
  mu_assert("湿度应在范围内", humidity >= 0.0f && humidity <= 100.0f);
  mu_assert("电压应在范围内", voltage >= 0.0f && voltage <= 5.0f);

  // 验证通道数量
  mu_assert("通道数量应为4", mapper.getChannelCount() == 4);

  return nullptr;
}

/**
 * @brief 测试单通道便利函数
 */
static const char *test_single_channel_convenience() {
  MockAdcHardware adcHardware;

  // 创建单通道ADC源
  int16_t offset = 100;
  AdcSource<1> adcSource(adcHardware, 12, &offset);

  // 使用便利函数创建温度映射器
  auto tempMapper =
      createTemperatureMapper(adcSource, -32768.0f, 32767.0f, -40.0f, 125.0f);

  float temp = tempMapper.getValue(0);

  // 验证结果
  mu_assert("单通道温度应在范围内", temp >= -40.0f && temp <= 125.0f);
  mu_assert("单通道数量应为1", tempMapper.getChannelCount() == 1);

  return nullptr;
}

/**
 * @brief 测试配置更新
 */
static const char *test_config_update() {
  MockAdcHardware adcHardware;

  int16_t offsets[2] = {0, 0};
  AdcSource<2> adcSource(adcHardware, 12, offsets);

  // 初始配置 - 映射到0-100范围
  LinearMapper<2>::Config initialConfig = {.inputMin = -32768.0f,
                                           .inputMax = 32767.0f,
                                           .outputMin = 0.0f,
                                           .outputMax = 100.0f,
                                           .clampOutput = true};

  LinearMapper<2> mapper(adcSource, initialConfig);

  float value1_before = mapper.getValue(0);

  // 更新通道0的配置 - 映射到0-1000范围
  LinearMapper<2>::Config newConfig = {.inputMin = -32768.0f,
                                       .inputMax = 32767.0f,
                                       .outputMin = 0.0f,
                                       .outputMax = 1000.0f,
                                       .clampOutput = true};

  mapper.updateChannelConfig(0, newConfig);

  float value1_after = mapper.getValue(0);
  float value2 = mapper.getValue(1); // 通道1配置未改变

  // 验证通道0的值变化了（因为映射范围变了）
  mu_assert("通道0配置更新后值应不同", value1_before != value1_after);
  mu_assert("通道1配置未改变", value2 >= 0.0f && value2 <= 100.0f);

  return nullptr;
}

/**
 * @brief 测试无效通道处理
 */
static const char *test_invalid_channel() {
  MockAdcHardware adcHardware;

  int16_t offsets[2] = {0, 0};
  AdcSource<2> adcSource(adcHardware, 12, offsets);

  LinearMapper<2>::Config config = {.inputMin = 0.0f,
                                    .inputMax = 1000.0f,
                                    .outputMin = 0.0f,
                                    .outputMax = 100.0f,
                                    .clampOutput = true};

  LinearMapper<2> mapper(adcSource, config);

  // 测试无效通道
  float invalidValue = mapper.getValue(5); // 通道5不存在

  mu_assert("无效通道应返回0", invalidValue == 0.0f);

  return nullptr;
}

} // namespace wibot

// 运行所有测试
static const char *all_tests() {
  mu_run_test(wibot::test_multichannel_linear_mapper);
  mu_run_test(wibot::test_single_channel_convenience);
  mu_run_test(wibot::test_config_update);
  mu_run_test(wibot::test_invalid_channel);
  return nullptr;
}

MINUNIT_MAIN(all_tests)