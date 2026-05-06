# wibotlib2 去模板化工作记录

## 背景

AQ-DJ03 在 Ozone 调试时变量查看不稳定，怀疑与 wibotlib2 中大量模板类型有关。目标是逐步把高层框架和当前项目实际使用的模板类改为普通类或显式运行时配置，提升调试器变量可见性和类型可读性。

本文件用于保存阶段性成果，后续每个类的子 agent 应先阅读并更新本文件。

## 当前用户决策

- `BootFrom<T>` 工作正常，不处理。
- `Thread<stack_size>` 改掉，改为非模板 `Thread`，线程栈由类内部动态分配。
- `ControllLoopTrgger<N>` 影响较小，不处理。
- 其他模板后续再议。

## 动态内存核实

当前工程使用 `THREADX`，但 CubeMX 生成的 ThreadX byte pool 动态内存模板没有启用：

- `AZURE_RTOS/App/app_azure_rtos.c` 的 `tx_application_define()` 没有创建 `TX_BYTE_POOL`。
- `tx_application_define()` 中的 `first_unused_memory` 当前未使用。
- 因此本轮不能直接使用 `tx_byte_allocate()` 作为线程栈 allocator。

当前可用的动态分配路径是 C/Picolibc heap：

- 工具链配置为 `STARM_PICOLIBC`。
- `malloc()` 通过 `Core/Src/sysmem.c` 中的 `_sbrk()` 从链接脚本 `_end` 向上增长。
- 堆增长上限是 `_estack - _Min_Stack_Size`。
- `STM32G031XX_FLASH.ld` 中 `_Min_Heap_Size = 0x200` 是链接期最小保留量，不是运行时硬上限。

注意：两个 1024 字节线程栈至少消耗 2048 字节动态堆。原先这些栈位于 `Thread<1024>` 对象内部的 `.bss` 中，改为动态分配后 RAM 总需求不会凭空减少，只是从静态对象转移到 heap。后续如果增加线程数量或栈大小，需要检查 map 文件和运行时堆余量。

## 已完成：`Thread<stack_size>`

### 修改文件

- `libs/wibotlib2/src/os/os.hpp`
- `libs/wibotlib2/src/port/os/threadx/os-port.tpp`
- `libs/wibotlib2/src/port/os/freertos/os-port.tpp`
- `libs/wibotlib2/src/port/os/nortos/os-port.tpp`
- `app/app.cpp`

### 接口变化

原接口：

```cpp
Thread<1024> mainLoopThread{"main", mainLoop, 6};
```

新接口：

```cpp
Thread mainLoopThread{"main", mainLoop, 6, 1024};
```

`Thread` 也保留默认栈大小：

```cpp
Thread thread{"name", worker, priority};
```

默认值为 `Thread::kDefaultStackSize == 1024`。

### 实现说明

- `Thread` 不再是模板类。
- `_stack` 改为 `u8*`。
- `_stackSize` 保存运行时栈大小。
- 构造函数内部调用 `std::malloc(stackSize)`。
- 析构函数先删除 RTOS thread，再调用 `std::free(_stack)`。
- ThreadX 路径已按当前项目优先实现并验证构建。
- FreeRTOS/NORTOS 的 tpp 也同步去模板化，保持接口一致。

## 当前 AQ-DJ03 使用点

当前应用中仍直接命中的模板类型：

- `app/app.cpp`
  - `ControllLoopTrgger<1> controlLoopTrigger`，按用户要求暂不处理。
  - `BootFrom<App> app`，按用户要求不处理。
- `app/ctrl.hpp`
  - `GpioDigitalSourceNode<1>`
  - `DigitalDebouncerNode<1>`
  - `KeyScanerNode<1>`
  - `PipelineChainBuilder<4>`
  - `PipelineChain<4>`
- `app/power.hpp`
  - `Binning<u16>`
  - `Buffer16<1>`

## 后续候选

后续如果继续去模板化，建议重新确认范围后再开子 agent：

- `GpioDigitalSourceNode<CHANNELS>`
- `DigitalDebouncerNode<CHANNELS>`
- `KeyScanerNode<CHANNELS>`
- `PipelineChain<MaxNodes>` / `PipelineChainBuilder<MaxNodes>`
- `Binning<T>`
- `Buffer<CAP, T>` / `Buffer16<CAP>`

## 执行规则

- 每个模板类先由独立子 agent 做只读分析，确认使用点、替代接口、风险和文件清单。
- 真正实现时每个子 agent 只负责一个类或一组强耦合类，避免交叉改动。
- 每轮实现后必须至少运行 `cmake --build --preset Debug`，若工具链不可用则记录失败原因。
- 每完成一个类，更新本文件的状态和迁移说明。

## 当前状态

- 2026-04-27：完成初步架构梳理和首批高层模板拆分。
- 2026-04-27：根据用户反馈收窄范围：`BootFrom<T>` 和 `ControllLoopTrgger<N>` 不处理；只实施 `Thread<stack_size>` 去模板化。
- 2026-04-27：`Thread<stack_size>` 已改为非模板 `Thread`，构造参数传入栈大小，内部通过 `malloc/free` 管理栈。
