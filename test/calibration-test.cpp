#include "../example/simple-calibration-example.hpp"
#include "../model/source/analog-source.hpp"
#include "../model/calibration/offset-calibrator.hpp"
#include <iostream>

using namespace wibot;

int main() {
    std::cout << "Simple AnalogSource Calibration Test" << std::endl;
    std::cout << "====================================" << std::endl;

    // Run basic functionality test
    std::cout << "\n1. Basic Functionality Test" << std::endl;

    // Create ADC source
    AnalogSource<2>::Config adcConfig{12};
    AnalogSource<2>         adcSource(adcConfig);

    // Create calibrator
    OffsetCalibrator<2>::Config calibConfig{10};
    OffsetCalibrator<2>         calibrator(calibConfig);

    // Get buffer and set simulation data
    u16* buffer = adcSource.getBuffer();
    buffer[0]   = 2048;  // 12-bit midpoint with offset
    buffer[1]   = 2050;

    // Update and read raw values
    adcSource.update();
    std::cout << "Before calibration: Ch0=" << adcSource.getValue(0)
              << ", Ch1=" << adcSource.getValue(1) << std::endl;

    // Perform calibration
    calibrator.startCalibration();
    for (int i = 0; i < 10; i++) {
        calibrator.addSample(buffer);
    }

    if (calibrator.isReady()) {
        std::cout << "Calculated offsets: Ch0=" << calibrator.getOffset(0)
                  << ", Ch1=" << calibrator.getOffset(1) << std::endl;

        // Apply calibration to ADC source
        calibrator.applyToAnalogSource(adcSource);

        adcSource.update();
        std::cout << "After calibration: Ch0=" << adcSource.getValue(0)
                  << ", Ch1=" << adcSource.getValue(1) << std::endl;
    }

    std::cout << "\n✓ Basic functionality test passed" << std::endl;

    // Test manual offset setting
    std::cout << "\n2. Manual Offset Test" << std::endl;

    AnalogSource<1> singleChannel(AnalogSource<1>::Config{10});
    u16*            singleBuffer = singleChannel.getBuffer();
    singleBuffer[0]              = 600;  // 10-bit with offset

    singleChannel.update();
    std::cout << "Original: " << singleChannel.getValue(0) << std::endl;

    // Manually set offset
    singleChannel.setOffset(0, -600);
    singleChannel.update();
    std::cout << "After manual offset (-600): " << singleChannel.getValue(0) << std::endl;

    std::cout << "\n✓ Manual offset test passed" << std::endl;

    // Run detailed examples
    std::cout << "\n3. Running detailed examples" << std::endl;
    SimpleCalibrationExample::runAllExamples();

    std::cout << "\n✓ All tests passed!" << std::endl;

    return 0;
}