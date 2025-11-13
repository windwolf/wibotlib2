# MemorySource 自动校准功能说明

## 功能概述

`MemorySource` 类新增了自动校准功能，可以自动检测并补偿ADC通道的系统偏差。该功能通过定时采样原始值，计算平均值后作为偏移量来消除固定的系统误差。

## 主要特性

### 1. 自动校准配置
```cpp
struct AutoCalibrationConfig {
    uint32_t sampleIntervalMs;  // 采样间隔时间（毫秒）
    uint16_t sampleCount;       // 采样次数
    bool     enabled;           // 是否启用自动校准
};
```

### 2. 校准机制
- **定时采样**: 按指定时间间隔获取原始ADC值
- **累计统计**: 对指定次数的采样值进行累加
- **平均计算**: 计算累计值的平均数作为系统偏差
- **自动补偿**: 将平均值的负值作为偏移量，实现零点校准
- **多通道独立**: 每个ADC通道都有独立的校准偏移量

### 3. 校准流程

```cpp
// 1. 配置自动校准参数
AutoCalibrationConfig calibConfig = {
    .sampleIntervalMs = 50,   // 每50ms采样一次
    .sampleCount = 20,        // 累计20次采样
    .enabled = true           // 启用自动校准
};

// 2. 开始自动校准
memorySource.startAutoCalibration(calibConfig);

// 3. 在主循环中持续调用update()
while (memorySource.isCalibrating()) {
    // 更新DMA缓冲区数据（由ADC自动完成）
    memorySource.update();  // 处理校准逻辑
    
    // 可选：显示校准进度
    float progress = memorySource.getCalibrationProgress();
    std::cout << "校准进度: " << progress << "%" << std::endl;
}

// 4. 校准完成，正常使用
// 后续的getValue()调用将返回校准后的值
```

## API 说明

### 校准控制方法

#### `startAutoCalibration(config)`
开始自动校准过程
- **参数**: `AutoCalibrationConfig` 校准配置
- **说明**: 重置校准状态，开始新的校准周期

#### `stopAutoCalibration()`
停止当前的校准过程
- **说明**: 立即停止校准，不保存当前的校准结果

#### `isCalibrating()`
检查是否正在校准中
- **返回**: `true` 正在校准，`false` 校准已完成或未开始

#### `getCalibrationProgress()`
获取校准进度
- **返回**: 当前采样进度百分比 (0-100%)

### 数据获取方法

所有原有的数据获取方法保持不变：
- `getValue(channel)`: 获取校准后的通道值
- `update()`: 更新数据并处理校准逻辑
- `setCalibration(offset)`: 设置全局校准偏移
- `getCalibration()`: 获取全局校准偏移

## 使用注意事项

### 1. 校准时机
- 建议在系统启动时进行校准
- 确保校准期间ADC输入稳定（如接地或固定电压）
- 避免在有信号输入时进行校准

### 2. 参数选择
- **采样间隔**: 根据系统更新频率调整，一般10-100ms
- **采样次数**: 建议10-50次，过多会延长校准时间
- **环境稳定**: 确保校准期间温度和电源稳定

### 3. 校准效果
- 每个通道独立校准，互不影响
- 校准值会自动应用到后续所有读数
- 可以与全局偏移量叠加使用

### 4. 实时性考虑
- 校准过程是非阻塞的，通过 `update()` 调用推进
- 校准期间仍可正常读取数据
- 外部需要提供时间基准（当前实现使用简单计数器）

## 典型应用场景

### 1. ADC零点校准
```cpp
// 系统启动时，输入端接地进行零点校准
AutoCalibrationConfig zeroCalib = {
    .sampleIntervalMs = 20,
    .sampleCount = 50,
    .enabled = true
};
memorySource.startAutoCalibration(zeroCalib);
```

### 2. 传感器基准校准
```cpp
// 传感器在已知基准状态下校准
// 例如：压力传感器在大气压下校准
AutoCalibrationConfig baselineCalib = {
    .sampleIntervalMs = 100,
    .sampleCount = 30,
    .enabled = true
};
memorySource.startAutoCalibration(baselineCalib);
```

### 3. 温漂补偿
```cpp
// 定期重新校准以补偿温度漂移
if (needRecalibration) {
    AutoCalibrationConfig driftCalib = {
        .sampleIntervalMs = 50,
        .sampleCount = 20,
        .enabled = true
    };
    memorySource.startAutoCalibration(driftCalib);
}
```

## 校准数据管理

校准结果以各通道独立的偏移值形式存储：
- 校准偏移值类型: `int16_t _channelOffsets[CHANNELS]`
- 偏移值范围: -32768 到 +32767
- 应用方式: `校准值 = 原始值 + 全局偏移 + 通道偏移`

校准数据不会自动保存到非易失性存储器，如需长期保存，应用程序需要：
1. 校准完成后读取偏移值
2. 保存到EEPROM/Flash等存储器
3. 系统重启后恢复偏移值