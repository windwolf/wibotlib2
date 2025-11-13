# STM32 项目尺寸优化指南

## 当前优化成果
- **总节省空间**: 1,016 字节 (约 1KB)
- **优化前**: ~94,272 字节
- **优化后**: 93,256 字节
- **Flash 使用率**: 71.15% (128KB 中的 93.2KB)

## 已实施的优化

### 1. 编译器优化标志
```cmake
-Os                    # 针对尺寸优化
-ffunction-sections    # 每个函数独立段
-fdata-sections        # 每个数据独立段
-flto                  # 链接时优化
```

### 2. 链接器优化标志
```cmake
--gc-sections          # 垃圾回收未使用段
--print-gc-sections    # 显示删除的段
--as-needed           # 仅链接必要库
--specs=nano.specs    # 使用精简版 C 库
```

### 3. 库模块精简
- CMSIS-DSP: 从 28 个模块精简到 4 个核心模块
- 移除的模块: FilteringFunctions, TransformFunctions, StatisticsFunctions 等

## 进一步优化建议

### A. HAL 库优化
在 `stm32g4xx_hal_conf.h` 中禁用不需要的 HAL 模块:
```c
// 仅保留项目需要的模块，注释掉其他
// #define HAL_ADC_MODULE_ENABLED
// #define HAL_CAN_MODULE_ENABLED  
// #define HAL_DAC_MODULE_ENABLED
// #define HAL_I2C_MODULE_ENABLED
// #define HAL_RTC_MODULE_ENABLED
```

### B. FreeRTOS 优化
在 `FreeRTOSConfig.h` 中优化配置:
```c
#define configTOTAL_HEAP_SIZE                8192    // 减少堆大小
#define configMINIMAL_STACK_SIZE            128     // 减少最小栈大小
#define configUSE_TRACE_FACILITY            0       // 禁用跟踪功能
#define configUSE_STATS_FORMATTING_FUNCTIONS 0      // 禁用统计功能
#define configSUPPORT_DYNAMIC_ALLOCATION    1       // 仅使用动态分配
#define configSUPPORT_STATIC_ALLOCATION     0       // 禁用静态分配
```

### C. 编译器额外优化
```cmake
-fno-unwind-tables      # 禁用展开表
-fno-asynchronous-unwind-tables  # 禁用异步展开表
-fomit-frame-pointer    # 省略帧指针
-fno-common             # 将未初始化的全局变量放在 BSS 段
```

### D. 链接器额外优化
```cmake
-Wl,--strip-debug       # 去除调试信息
-Wl,--relax             # 链接器松弛优化
-Wl,--orphan-handling=warn  # 警告孤立段
```

### E. 代码级优化
1. **使用 const 关键字**: 将只读数据放在 Flash 中
2. **避免浮点运算**: 使用整数或定点运算
3. **减少全局变量**: 使用局部变量和栈
4. **优化字符串**: 使用 PROGMEM 存储常量字符串

### F. Release 构建优化
切换到 Release 构建模式:
```bash
cmake --build build/Release
```

Release 模式额外包含:
- `-g0`: 无调试信息
- 更激进的 LTO 优化

## 预期效果
通过以上优化，预计可以再节省 2-5KB 空间，总优化效果可达 3-6KB。

## 监控工具
- 使用 `--print-gc-sections` 监控死代码消除
- 使用 `--print-memory-usage` 查看内存使用
- 生成 map 文件分析最大的代码段

## 注意事项
- 在 Release 模式下测试所有功能
- 某些优化可能影响调试体验
- 保留备份配置用于开发调试