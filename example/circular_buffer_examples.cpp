/**
 * @file circular_buffer_examples.cpp
 * @brief CircularBuffer 完整使用示例
 *
 * 本文件展示了 CircularBuffer 在实际项目中的各种应用场景：
 * 1. 基础读写操作
 * 2. UART DMA 接收
 * 3. 音频数据处理
 * 4. 传感器数据采样
 * 5. 协议解析
 */

#include "buffer.hpp"
#include "circular-buffer.hpp"
#include "logger.hpp"
#include "uart.hpp"


LOGGER("cb_example")

using namespace wibot;

// ==================== 示例 1: 基础操作 ====================

/**
 * @brief 演示循环缓冲区的基本操作
 */
void example1_BasicOperations() {
  LOG_I("=== Example 1: Basic Operations ===");

  // 创建 256 字节的循环缓冲区
  Buffer<256> storage;
  CircularBuffer8 cb(storage);

  // 检查初始状态
  LOG_I("Capacity: %d", cb.getCapacity());
  LOG_I("Empty: %d", cb.isEmpty());
  LOG_I("Size: %d", cb.getSize());

  // 写入数据
  u8 writeData[] = {0x01, 0x02, 0x03, 0x04, 0x05};
  u32 written = cb.write(writeData, 5);
  LOG_I("Written: %d bytes", written);
  LOG_I("Size after write: %d", cb.getSize());

  // 读取数据
  u8 readData[10];
  u32 read = cb.read(readData, 3);
  LOG_I("Read: %d bytes", read);
  LOG_I("Data: 0x%02X 0x%02X 0x%02X", readData[0], readData[1], readData[2]);
  LOG_I("Size after read: %d", cb.getSize());

  // 查看数据（不移除）
  u8 peekData[2];
  u32 peeked = cb.peek(peekData, 0, 2);
  LOG_I("Peeked: %d bytes", peeked);
  LOG_I("Peek data: 0x%02X 0x%02X", peekData[0], peekData[1]);
  LOG_I("Size after peek: %d (unchanged)", cb.getSize());

  // 清空缓冲区
  cb.clear();
  LOG_I("Cleared. Empty: %d", cb.isEmpty());
}

// ==================== 示例 2: UART DMA 接收 ====================

/**
 * @brief UART DMA 接收缓冲区
 *
 * 演示如何使用循环缓冲区配合 UART DMA 实现高效的数据接收
 */
class UartDmaReceiver {
public:
  UartDmaReceiver(UART_HandleTypeDef &huart)
      : huart_(huart), rxBuffer_(storage_) {}

  /**
   * @brief 初始化 DMA 接收
   */
  void init() {
    LOG_I("=== Example 2: UART DMA Receiver ===");

    // 获取缓冲区写入指针和可用空间
    u8 *writePtr = rxBuffer_.getWritePtr();
    u32 space = rxBuffer_.getSpace();

    LOG_I("Starting DMA receive: ptr=%p, space=%d", writePtr, space);

    // 启动 UART DMA 接收（实际项目中的代码）
    // HAL_UART_Receive_DMA(&huart_, writePtr, space);
  }

  /**
   * @brief DMA 半完成回调
   *
   * 当 DMA 完成一半数据传输时调用
   */
  void onDmaHalfComplete() {
    u32 halfSize = rxBuffer_.getCapacity() / 2;

    LOG_I("DMA Half Complete: %d bytes", halfSize);

    // 虚拟写入（仅移动指针）
    bool overflow = rxBuffer_.writeVirtual(halfSize);

    if (overflow) {
      LOG_W("Buffer overflow detected!");
    }

    // 处理接收到的数据
    processReceivedData();
  }

  /**
   * @brief DMA 完成回调
   *
   * 当 DMA 完成全部数据传输时调用
   */
  void onDmaComplete() {
    u32 halfSize = rxBuffer_.getCapacity() / 2;

    LOG_I("DMA Complete: %d bytes", halfSize);

    // 虚拟写入
    bool overflow = rxBuffer_.writeVirtual(halfSize);

    if (overflow) {
      LOG_W("Buffer overflow detected!");
    }

    // 处理接收到的数据
    processReceivedData();

    // 重启 DMA
    init();
  }

  /**
   * @brief 模拟数据接收（用于演示）
   */
  void simulateReceive() {
    LOG_I("Simulating UART data reception...");

    // 模拟接收一些数据
    u8 data[] = "Hello from UART!\r\n";
    rxBuffer_.write(data, sizeof(data) - 1);

    // 处理数据
    processReceivedData();

    // 模拟更多数据
    u8 data2[] = "More data...\r\n";
    rxBuffer_.write(data2, sizeof(data2) - 1);

    processReceivedData();
  }

  /**
   * @brief 读取一行数据（以 \n 结束）
   */
  bool readLine(char *line, u32 maxLen) {
    u32 size = rxBuffer_.getSize();

    // 查找换行符
    for (u32 i = 0; i < size && i < maxLen - 1; i++) {
      u8 *ptr = rxBuffer_.peekPtr(i);
      if (ptr && (*ptr == '\n' || *ptr == '\r')) {
        // 找到行结束符
        u32 lineLen = i + 1;
        rxBuffer_.read((u8 *)line, lineLen);
        line[lineLen] = '\0';
        return true;
      }
    }

    return false;
  }

private:
  void processReceivedData() {
    char line[128];

    while (readLine(line, sizeof(line))) {
      LOG_I("Received line: %s", line);
    }

    LOG_I("Buffer size: %d", rxBuffer_.getSize());
  }

  UART_HandleTypeDef &huart_;
  Buffer<256> storage_;
  CircularBuffer8 rxBuffer_;
};

// ==================== 示例 3: 音频数据处理 ====================

/**
 * @brief 音频数据缓冲和处理
 *
 * 演示如何使用循环缓冲区处理音频流
 */
class AudioProcessor {
public:
  static constexpr u32 SAMPLE_RATE = 16000; // 16kHz
  static constexpr u32 FRAME_SIZE = 256;    // 256 samples per frame

  AudioProcessor() : audioBuffer_(storage_) {}

  /**
   * @brief ADC 采样回调
   */
  void onAdcSample(i16 sample) {
    // 写入采样值（覆盖模式）
    audioBuffer_.write(&sample, 1, true);
  }

  /**
   * @brief 处理音频帧
   */
  void process() {
    // 检查是否有完整的帧
    if (audioBuffer_.getSize() >= FRAME_SIZE) {
      i16 frame[FRAME_SIZE];

      // 读取一帧数据
      u32 read = audioBuffer_.read(frame, FRAME_SIZE);

      if (read == FRAME_SIZE) {
        // 处理音频帧
        processFrame(frame, FRAME_SIZE);
      }
    }
  }

  /**
   * @brief 模拟音频处理
   */
  void simulate() {
    LOG_I("=== Example 3: Audio Processing ===");

    // 生成模拟音频数据
    LOG_I("Generating audio samples...");
    for (u32 i = 0; i < FRAME_SIZE * 3; i++) {
      // 生成正弦波
      f32 angle = (f32)i / SAMPLE_RATE * 2.0f * 3.14159f * 440.0f; // 440Hz
      i16 sample = (i16)(sin(angle) * 16000.0f);

      onAdcSample(sample);
    }

    // 处理音频
    LOG_I("Processing audio frames...");
    while (audioBuffer_.getSize() >= FRAME_SIZE) {
      process();
    }

    LOG_I("Remaining samples: %d", audioBuffer_.getSize());
  }

private:
  void processFrame(i16 *samples, u32 count) {
    // 计算音频帧的统计信息
    i32 sum = 0;
    i16 maxVal = samples[0];
    i16 minVal = samples[0];

    for (u32 i = 0; i < count; i++) {
      sum += samples[i];
      if (samples[i] > maxVal)
        maxVal = samples[i];
      if (samples[i] < minVal)
        minVal = samples[i];
    }

    i16 avg = sum / count;

    LOG_I("Audio frame: avg=%d, max=%d, min=%d, peak-to-peak=%d", avg, maxVal,
          minVal, maxVal - minVal);
  }

  Buffer<4096, i16> storage_; // 4096 samples buffer
  CircularBuffer<i16> audioBuffer_;
};

// ==================== 示例 4: 传感器数据采样 ====================

/**
 * @brief 传感器数据缓冲和统计
 */
class SensorDataBuffer {
public:
  struct SensorSample {
    u32 timestamp;
    f32 temperature;
    f32 humidity;
    f32 pressure;
  };

  SensorDataBuffer() : sampleBuffer_(storage_) {}

  /**
   * @brief 添加传感器采样
   */
  void addSample(u32 timestamp, f32 temp, f32 humid, f32 press) {
    SensorSample sample = {timestamp, temp, humid, press};

    // 使用覆盖模式保持最新数据
    sampleBuffer_.write(&sample, 1, true);
  }

  /**
   * @brief 获取平均值
   */
  SensorSample getAverage() {
    u32 count = sampleBuffer_.getSize();

    if (count == 0) {
      return {0, 0.0f, 0.0f, 0.0f};
    }

    SensorSample sum = {0, 0.0f, 0.0f, 0.0f};

    // 使用 peek 计算平均值（不移除数据）
    for (u32 i = 0; i < count; i++) {
      SensorSample *sample = sampleBuffer_.peekPtr(i);
      if (sample) {
        sum.temperature += sample->temperature;
        sum.humidity += sample->humidity;
        sum.pressure += sample->pressure;
      }
    }

    return {0, sum.temperature / count, sum.humidity / count,
            sum.pressure / count};
  }

  /**
   * @brief 获取最大值
   */
  SensorSample getMax() {
    u32 count = sampleBuffer_.getSize();

    if (count == 0) {
      return {0, 0.0f, 0.0f, 0.0f};
    }

    SensorSample *first = sampleBuffer_.peekPtr(0);
    SensorSample max = *first;

    for (u32 i = 1; i < count; i++) {
      SensorSample *sample = sampleBuffer_.peekPtr(i);
      if (sample) {
        if (sample->temperature > max.temperature)
          max.temperature = sample->temperature;
        if (sample->humidity > max.humidity)
          max.humidity = sample->humidity;
        if (sample->pressure > max.pressure)
          max.pressure = sample->pressure;
      }
    }

    return max;
  }

  /**
   * @brief 模拟传感器采样
   */
  void simulate() {
    LOG_I("=== Example 4: Sensor Data Sampling ===");

    // 模拟 100 个采样
    LOG_I("Collecting sensor samples...");
    for (u32 i = 0; i < 100; i++) {
      f32 temp = 25.0f + (i % 20) * 0.1f - 1.0f;    // 24-26°C
      f32 humid = 60.0f + (i % 30) * 0.5f - 7.5f;   // 52.5-67.5%
      f32 press = 1013.0f + (i % 10) * 0.2f - 1.0f; // 1012-1014 hPa

      addSample(i * 1000, temp, humid, press);
    }

    LOG_I("Collected %d samples", sampleBuffer_.getSize());

    // 计算统计信息
    SensorSample avg = getAverage();
    LOG_I("Average: T=%.2f°C, H=%.2f%%, P=%.2fhPa", avg.temperature,
          avg.humidity, avg.pressure);

    SensorSample max = getMax();
    LOG_I("Maximum: T=%.2f°C, H=%.2f%%, P=%.2fhPa", max.temperature,
          max.humidity, max.pressure);
  }

private:
  Buffer<128, SensorSample> storage_;
  CircularBuffer<SensorSample> sampleBuffer_;
};

// ==================== 示例 5: 协议解析 ====================

/**
 * @brief 简单协议解析器
 *
 * 协议格式: [0xAA][0x55][LEN][DATA...][CHECKSUM]
 */
class ProtocolParser {
public:
  static constexpr u8 HEADER1 = 0xAA;
  static constexpr u8 HEADER2 = 0x55;

  ProtocolParser() : dataBuffer_(storage_) {}

  /**
   * @brief 添加接收数据
   */
  void feedData(const u8 *data, u32 length) {
    dataBuffer_.write(data, length, false);
  }

  /**
   * @brief 解析数据包
   */
  bool parsePacket(u8 *payload, u32 maxLen, u32 &outLen) {
    // 至少需要 5 字节 (2 header + 1 len + 1 data + 1 checksum)
    if (dataBuffer_.getSize() < 5) {
      return false;
    }

    // 查找帧头
    u32 headerPos = 0;
    bool foundHeader = false;

    for (u32 i = 0; i < dataBuffer_.getSize() - 1; i++) {
      u8 *ptr1 = dataBuffer_.peekPtr(i);
      u8 *ptr2 = dataBuffer_.peekPtr(i + 1);

      if (ptr1 && ptr2 && *ptr1 == HEADER1 && *ptr2 == HEADER2) {
        headerPos = i;
        foundHeader = true;
        break;
      }
    }

    if (!foundHeader) {
      // 丢弃无效数据
      u32 size = dataBuffer_.getSize();
      if (size > 1) {
        dataBuffer_.readVirtual(size - 1);
      }
      return false;
    }

    // 丢弃帧头之前的数据
    if (headerPos > 0) {
      dataBuffer_.readVirtual(headerPos);
    }

    // 读取长度
    u8 *lenPtr = dataBuffer_.peekPtr(2);
    if (!lenPtr)
      return false;

    u8 payloadLen = *lenPtr;
    u32 packetLen = 2 + 1 + payloadLen + 1; // header + len + payload + checksum

    if (dataBuffer_.getSize() < packetLen) {
      return false; // 数据不完整
    }

    // 读取整个数据包
    u8 packet[256];
    dataBuffer_.read(packet, packetLen);

    // 验证校验和
    u8 checksum = 0;
    for (u32 i = 0; i < packetLen - 1; i++) {
      checksum ^= packet[i];
    }

    if (checksum != packet[packetLen - 1]) {
      LOG_W("Checksum mismatch");
      return false;
    }

    // 提取有效载荷
    if (payloadLen > maxLen) {
      LOG_W("Payload too large");
      return false;
    }

    memcpy(payload, &packet[3], payloadLen);
    outLen = payloadLen;

    return true;
  }

  /**
   * @brief 模拟协议解析
   */
  void simulate() {
    LOG_I("=== Example 5: Protocol Parser ===");

    // 构造测试数据包
    u8 packet1[] = {0xAA, 0x55, 0x05, 0x01, 0x02, 0x03, 0x04, 0x05, 0};
    packet1[8] = 0;
    for (int i = 0; i < 8; i++) {
      packet1[8] ^= packet1[i]; // 计算校验和
    }

    u8 packet2[] = {0xAA, 0x55, 0x03, 0x10, 0x20, 0x30, 0};
    packet2[6] = 0;
    for (int i = 0; i < 6; i++) {
      packet2[6] ^= packet2[i];
    }

    // 添加一些噪声
    u8 noise[] = {0xFF, 0xFE, 0xFD};
    feedData(noise, sizeof(noise));

    // 添加数据包1
    feedData(packet1, sizeof(packet1));

    // 添加数据包2
    feedData(packet2, sizeof(packet2));

    // 解析数据包
    u8 payload[256];
    u32 payloadLen;

    int packetCount = 0;
    while (parsePacket(payload, sizeof(payload), payloadLen)) {
      packetCount++;
      LOG_I("Packet %d: length=%d", packetCount, payloadLen);

      // 打印有效载荷
      LOG_I("  Payload: ");
      for (u32 i = 0; i < payloadLen; i++) {
        LOG_I("0x%02X ", payload[i]);
      }
      LOG_I("\n");
    }

    LOG_I("Total packets parsed: %d", packetCount);
  }

private:
  Buffer<512> storage_;
  CircularBuffer8 dataBuffer_;
};

// ==================== 主函数 ====================

/**
 * @brief 运行所有示例
 */
void runAllExamples() {
  // 示例 1: 基础操作
  example1_BasicOperations();
  printf("\n");

  // 示例 2: UART DMA 接收（需要实际硬件）
  // UartDmaReceiver receiver(huart1);
  // receiver.simulate();
  printf("\n");

  // 示例 3: 音频处理
  AudioProcessor audioProc;
  audioProc.simulate();
  printf("\n");

  // 示例 4: 传感器数据采样
  SensorDataBuffer sensorBuffer;
  sensorBuffer.simulate();
  printf("\n");

  // 示例 5: 协议解析
  ProtocolParser parser;
  parser.simulate();
  printf("\n");

  LOG_I("=== All Examples Completed ===");
}

/**
 * @brief 应用入口点（在实际项目中调用）
 */
extern "C" void circularBufferExamples() { runAllExamples(); }
