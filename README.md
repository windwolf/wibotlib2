# WibotLib - STM32 Embedded Development Library

## Overview

WibotLib is a modern C++ embedded development library designed for STM32 microcontroller series, providing a complete hardware abstraction layer, real-time operating system support, and rich device drivers. The library is built to work with STM32CubeMX generated CMake projects, adopts a modular design, supports multiple RTOS, and provides a unified API interface, greatly simplifying embedded system development.

## Core Features

### 🚀 **Multi-RTOS Support**
- Full support for FreeRTOS and ThreadX real-time operating systems
- Unified operating system abstraction layer, allowing seamless switching between different RTOS without code modification
- Support for bare-metal (No-OS) operation mode

### 🔄 **Modern Asynchronous Programming Model**
- EventGroup-based asynchronous processing mechanism
- AsyncSource/AsyncResult pattern for elegant asynchronous operation handling
- Resource pool management to avoid memory fragmentation

### 🎛️ **Data Flow Pipeline Architecture**
- SyncPipeline-based data processing framework
- Chainable composition of data sources, filters, mappers, and controllers
- Multi-channel parallel processing support

### 🔧 **Rich Hardware Abstraction Layer (HAL)**
- Complete hardware abstraction for STM32 series chips
- Unified peripheral interfaces to simplify hardware operations
- Support for advanced features like DMA and interrupts

### 📡 **Comprehensive Device Driver Support**
- Display devices: SSD1306, ST7735, ST77xx series OLED/TFT displays
- Communication modules: LoRa wireless modules (Rola-E22)
- Sensors: Multi-channel ADC sensor support
- Storage devices: Flash, EEPROM and other storage components
- Real-time clock: RX8010 and other RTC chips

### 📋 **Rich Protocol Stack**
- **GNSS Positioning**: Support for NMEA, UBX, CASIC and other GNSS protocols
- **Camera Interface**: OV7725 and other camera module support
- **Motor Control**: DShot protocol support
- **General Protocols**: CRC verification, data encoding/decoding, etc.

## Architecture Design

### Layered Architecture

```
┌─────────────────────────────────────────────┐
│              Application Layer               │
├─────────────────────────────────────────────┤
│            Device Driver Layer               │
│  Display│Storage│Motor│RF│RTC│IO│Sensors     │
├─────────────────────────────────────────────┤
│             Protocol Layer                   │
│     GNSS │ Camera │ DShot │ CRC             │
├─────────────────────────────────────────────┤
│            Data Model Layer                  │
│  Source│Filter│Mapper│Adapter│Controller│RW  │
├─────────────────────────────────────────────┤
│        Hardware Abstraction Layer           │
│    GPIO │ SPI │ I2C │ UART │ ADC │ PWM     │
├─────────────────────────────────────────────┤
│        Operating System Port Layer          │
│    FreeRTOS │ ThreadX │ No-OS              │
├─────────────────────────────────────────────┤
│             Base Components                  │
│  Types│Buffer│Time│Math│LinkedList│Async    │
└─────────────────────────────────────────────┘
```

### Core Components

#### 1. Base Components
- **Type System**: Unified data type definitions (u8, u16, u32, f32, etc.)
- **Buffer Management**: Efficient circular buffers and memory management
- **Time System**: High-precision timing and delay functions
- **Mathematical Operations**: Optimized math function library
- **Result Processing**: Unified Result error handling mechanism

#### 2. Operating System Abstraction Layer (OS)
```cpp
// Cross-platform thread creation
Thread<1024> worker_thread("worker", &worker, 5);

// Unified synchronization primitives
Mutex data_mutex("data");
EventGroup events("events");
MessageQueue queue("queue", buffer, 32, 10);

// Asynchronous programming support
AsyncSource source;
AsyncResult result = source.getResult();
```

#### 3. Hardware Abstraction Layer (HAL)
```cpp
// Unified peripheral interfaces
SpiMaster spi(SPI1);
I2cMaster i2c(I2C1);
AdcChannel adc(ADC1, ADC_CHANNEL_1);

// System-level functionality
System::delayUs(100);
u32 freq = System::getSysClockFreq();
```

#### 4. Data Flow Pipeline
```cpp
// Build data processing pipeline
AdcSourcePipeline adcSource;
LowPassFilter filter(&adcSource);
LinearMapper mapper(&filter);
PidController controller(&mapper);

// Chain updates
controller.update();
f32 output = controller.getValue(0);
```

## Getting Started

### Requirements

- **Project Base**: STM32CubeMX generated CMake project
- **Toolchain**: ARM GCC or Clang
- **Build System**: CMake 3.22+
- **RTOS**: FreeRTOS 10.0+ or ThreadX 6.0+
- **Chip Support**: STM32G4 series (other series continuously expanding)

### Prerequisites

WibotLib is designed to work with STM32CubeMX generated projects that use CMake as the build system. Before using WibotLib:

1. **Create STM32CubeMX Project**:
   - Use STM32CubeMX to configure your target STM32 chip
   - Enable required peripherals (GPIO, SPI, I2C, UART, ADC, etc.)
   - Configure RTOS if needed (FreeRTOS or ThreadX)
   - Generate code with **CMake** toolchain (not Makefile or other IDEs)

2. **Project Structure**:
   Your STM32CubeMX generated project should have this structure:
   ```
   your_project/
   ├── CMakeLists.txt          # Main CMake file
   ├── cmake/
   │   └── stm32cubemx/        # STM32CubeMX CMake files
   ├── Core/                   # Generated HAL code
   │   ├── Inc/
   │   └── Src/
   ├── Drivers/                # STM32 HAL drivers
   └── Middlewares/            # RTOS and other middleware
   ```

3. **Add WibotLib**:
   - Clone or add WibotLib as a subdirectory in your project
   - WibotLib will automatically integrate with the STM32CubeMX generated CMake configuration

### Basic Usage

1. **Include Headers**
```cpp
#include "wibotlib.hpp"  // Include all core functionality
```

2. **Initialize System**
```cpp
// In main function
wibot::System::init();
```

3. **Create Threads**
```cpp
class MyWorker : public wibot::Worker {
public:
    void run() override {
        while (true) {
            // Work logic
            wibot::os::sleep(100);
        }
    }
};

MyWorker worker;
wibot::Thread<1024> thread("worker", &worker, 5);
thread.start();
```

4. **Use Device Drivers**
```cpp
// SSD1306 OLED Display
wibot::SSD1306 display(i2c, 0x3C);
display.init();
display.print("Hello WibotLib!");
display.refresh();

// LoRa Communication Module
wibot::RolaE22 lora(uart, gpio_m0, gpio_m1);
lora.init();
lora.sendData(data, sizeof(data));
```

## Device Support

### Display Devices
- **SSD1306**: OLED Display (I2C/SPI)
- **ST7735/ST77xx**: TFT Color Displays
- **TM1652**: 7-Segment Display Driver

### Communication Devices
- **Rola-E22**: LoRa Long Range Communication Module
- **OV7725**: Camera Module

### Sensors & Actuators
- **Multi-channel ADC**: Analog signal acquisition
- **RTC Module**: Real-time clock (RX8010)
- **Motor Control**: DShot protocol ESC support

### Storage Devices
- **Flash Storage**: Internal/External Flash operations
- **EEPROM**: Non-volatile data storage

## GNSS Positioning Support

WibotLib provides a complete GNSS positioning solution:

### Supported Protocols
- **NMEA 0183**: Standard GPS protocol
- **UBX**: u-blox proprietary protocol
- **CASIC**: CASIC protocol

### Features
```cpp
// NMEA parsing example
wibot::NmeaParser parser;
wibot::GnssData gnssData;

if (parser.parse(sentence, gnssData)) {
    f64 lat = gnssData.latitude;
    f64 lon = gnssData.longitude;
    f32 alt = gnssData.altitude;
}
```

## Data Processing Pipeline

### Pipeline Types

#### Data Sources
- **ConstantSource**: Constant data source
- **DigitalSource**: Digital signal source
- **AnalogSource**: Analog signal source

#### Filters
- **LowPassFilter**: Low-pass filter
- **MedianFilter**: Median filter
- **CustomFilter**: Custom filter

#### Mappers
- **LinearMapper**: Linear mapping
- **PiecewiseLinearMapper**: Piecewise linear mapping
- **CustomMapper**: Custom mapping

#### Controllers
- **PidController**: PID controller with 1-4 channel support

### Usage Example

```cpp
// Build temperature control system
AdcSource tempSensor;           // Temperature sensor
LowPassFilter filter(&tempSensor);  // Noise filtering
LinearMapper mapper(&filter);       // Voltage to temperature conversion
PidController<1> heaterCtrl(&mapper); // PID temperature control

// Configure PID parameters
PidControllerConfig config;
config.Kp = 2.0f;
config.Ki = 0.5f; 
config.Kd = 0.1f;
config.setPoint = 25.0f; // Target temperature 25°C
heaterCtrl.setConfig(config);

// Control loop
while (true) {
    heaterCtrl.update();
    f32 heaterPower = heaterCtrl.getValue(0);
    pwm.setDutyCycle(heaterPower);
    wibot::os::sleep(100);
}
```

## Asynchronous Programming

WibotLib provides modern asynchronous programming support:

```cpp
// Asynchronous data reading
AsyncSource source;
AsyncResult result = source.getResult();

// Start async operation
dma.startRead(buffer, size);

// Wait for completion
Result status = result.wait(1000); // Wait up to 1000ms
if (status.isOk()) {
    // Process received data
} else if (status.isTimeout()) {
    // Handle timeout
}
```

## Project Configuration

### CMake Integration

WibotLib integrates seamlessly with STM32CubeMX generated CMake projects. In your main CMakeLists.txt:

```cmake
# Your STM32CubeMX generated project setup
cmake_minimum_required(VERSION 3.22)
project(your_project_name)

# STM32CubeMX generated configurations
add_subdirectory(cmake/stm32cubemx)

# Add WibotLib subdirectory
add_subdirectory(wibotlib)

# Create your executable
add_executable(${CMAKE_PROJECT_NAME} 
    # Your application sources
    app/main.cpp
    app/app.cpp
)

# Link WibotLib to your project
target_link_libraries(${CMAKE_PROJECT_NAME}
    wibotlib_impl     # WibotLib implementation
    wibotlib          # WibotLib interface
    stm32cubemx       # STM32CubeMX generated code
)

# Include WibotLib headers
target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE
    wibotlib/src
    app
)
```

### Integration Notes

- WibotLib automatically detects and integrates with STM32CubeMX generated `stm32cubemx` target
- The library uses the same HAL drivers and system configuration as your CubeMX project
- RTOS configuration from CubeMX is automatically recognized and used

### Compilation Options

```cmake
# C++23 standard support
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Optimization options
target_compile_options(wibotlib INTERFACE 
    -Os          # Size optimization
    -ffunction-sections
    -fdata-sections
)
```

## Documentation Resources

Detailed documentation is located in the `docs/` directory:

- [Asynchronous Programming Guide](docs/async-result-usage.md)
- [Data Pipeline Details](docs/pipeline.md) 
- [PID Controller Usage](docs/README.md)
- [ADC Usage Guide](docs/analog-input-README.md)
- [Digital IO Control](docs/digital-source-README.md)
- [Filter Configuration](docs/lowpass-filter-README.md)
- [Auto-Calibration System](docs/AutoCalibration_Usage_Guide.md)

## License

This project is licensed under the MIT License. See the LICENSE file for details.

## Contributing

Welcome to submit Issues and Pull Requests to improve WibotLib. Please ensure:

1. Code style follows project conventions
2. Add appropriate unit tests
3. Update relevant documentation

## Technical Support

- **GitHub Issues**: Report bugs and feature requests
- **Documentation**: Check the docs directory for detailed usage instructions
- **Example Code**: Refer to complete examples in the example directory

---

**WibotLib - Making STM32 embedded development simpler and more modern!**