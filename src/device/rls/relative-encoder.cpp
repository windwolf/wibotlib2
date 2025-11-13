// //
// // Created by zhouj on 2023/10/9.
// //

// #include "relative-encoder.hpp"

// #include "logger.hpp"
// LOGGER("enc")

// namespace wibot {

// void RelativeEncoder::updatePositionValue(u32 value) {
//     i32 delta = (i32)value - (i32)lastValue_;
//     // LOG_D_INTERVAL(100, "delta:%ld", delta);
//     if (delta < -(i32)_config.wrapRange / 2) {
//         delta += (i32)_config.wrapRange;
//     } else if (delta > (i32)_config.wrapRange / 2) {
//         delta -= (i32)_config.wrapRange;
//     }
//     position_ += delta;
//     speed_     = filteredSpeed_.filter(delta / _config.sampleTime);
//     lastValue_ = value;
// }
// void RelativeEncoder::setConfig(RelativeEncoderConfig& config) {
//     this->_config = config;
//     FirstOrderLowPassFilterConfig cfg;
//     cfg.sampleTime = config.sampleTime;
//     cfg.wrapValue  = 0;
//     // According to the formula BW*RiseTime=0.35,
//     cfg.cutoffFreq = 0.35f / (1.0f / config.maxSpeed);
//     filteredSpeed_.setConfig(cfg);
//     filteredSpeed_.reset();
// }
// void RelativeEncoder::reset(f32 position, f32 speed) {
//     position_  = position;
//     lastValue_ = position;
//     speed_     = speed;
//     filteredSpeed_.reset(speed_);
// }

// }  // namespace wibot
