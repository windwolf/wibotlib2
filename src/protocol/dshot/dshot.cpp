//
// Created by zhouj on 2025/5/14.
//

#include "dshot.hpp"
#include "peripheral.hpp"

#include "system.hpp"

#if defined(HAL_TIM_MODULE_ENABLED)

namespace wibot {
DShot::DShot(TIM_HandleTypeDef* tim, DShotProtocol protocol) : _tim(tim), _protocol(protocol) {
    PeripheralManager::getInstance().registerPeripheral(this, tim);
    HAL_TIM_RegisterCallback(tim, HAL_TIM_PWM_PULSE_FINISHED_CB_ID, onCplt);
    HAL_TIM_RegisterCallback(tim, HAL_TIM_ERROR_CB_ID, onError);

    // 根据TIM外设编号, 获取TIM的总线始终频率
    u32 timerClock  = System::getTIMFreq(tim->Instance);
    // DShot位持续时间 (µs)
    f32 bitDuration = 0;

    // 0值和1值的比例 (DShot协议中, 1的脉宽是0的脉宽的约2.5倍)
    // 使用整体时间的37.5%(0值) 和 75%(1值) 进行区分
    const f32 dutyCycle0 = 0.375f;
    const f32 dutyCycle1 = 0.75f;

    switch (_protocol) {
        case DShotProtocol::DShot150:
            bitDuration = 6.67f;  // 1/150kHz * 1000 (µs)
            break;
        case DShotProtocol::DShot300:
            bitDuration = 3.33f;  // 1/300kHz * 1000 (µs)
            break;
        case DShotProtocol::DShot600:
            bitDuration = 1.67f;  // 1/600kHz * 1000 (µs)
            break;
        default:
            // 默认使用DShot150
            bitDuration = 6.67f;
            break;
    }

    // 计算定时器的频率, 使定时器周期足够高以提供良好的分辨率
    // 我们的目标是使每个位有足够的分辨率, 比如约100个时钟周期
    u32 targetTimerFreq = (u32)(1000000.0f / bitDuration * 100);
    // 计算预分频器, 使定时器频率接近目标频率
    u32 prescaler       = timerClock / targetTimerFreq;
    if (prescaler < 1) prescaler = 1;
    // 计算实际的定时器频率
    u32 actualTimerFreq = timerClock / prescaler;
    // 计算定时器周期 (一个位的定时器计数值)
    u32 period          = (u32)(actualTimerFreq * bitDuration / 1000000.0f);
    // 计算0值和1值的脉宽计数
    u32 pulse0          = (u32)(period * dutyCycle0);
    u32 pulse1          = (u32)(period * dutyCycle1);

    // 配置定时器

    LL_TIM_SetPrescaler(_tim->Instance, prescaler - 1);
    LL_TIM_SetAutoReload(_tim->Instance, period - 1);

    // 存储脉宽值以便在send函数中使用
    _pulse0 = pulse0;
    _pulse1 = pulse1;

    // 初始化DMA和其他必要的配置
    // ...
}

DShot::~DShot() {
    PeripheralManager::getInstance().unregisterPeripheral(this);
}

u16 add_checksum(u16 packet_telemetry) {
    u8  i;
    int csum      = 0;
    int csum_data = packet_telemetry;

    for (i = 0; i < 3; i++) {
        csum ^= csum_data;  // xor data by nibbles
        csum_data >>= 4;
    }
    csum &= 0xf;
    packet_telemetry = (packet_telemetry << 4) | csum;

    return packet_telemetry;  //append checksum
}

AsyncResult DShot::send(u8 channel, u16 command, bool telemetry) {
    u32 halChannel = 0;
    switch (command) {
        case 1:
            halChannel = TIM_CHANNEL_1;
            break;
        case 2:

            halChannel = TIM_CHANNEL_2;
            break;
        case 3:
            halChannel = TIM_CHANNEL_3;
            break;
        case 4:
            halChannel = TIM_CHANNEL_4;
            break;
#ifdef TIM_CHANNEL_5
        case 5:
            halChannel = TIM_CHANNEL_5;
            break;
#endif
#ifdef TIM_CHANNEL_6
        case 6:
            halChannel = TIM_CHANNEL_6;
            break;
#endif
        default:
            return AsyncResult::fromResult(Result::kInvalidParameter);
    }

    // 计算数据帧
    u16 data = (command << 1) | telemetry;

    data = add_checksum(data);

    // 将数据转换为脉宽值

    for (int i = 0; i < 16; i++) {
        _framebuffer[i] = ((data >> (15 - i)) & 1) ? _pulse1 : _pulse0;
    }
    _framebuffer[16] = 0;

    auto rst = HAL_TIM_PWM_Start_DMA(_tim, halChannel, (u32*)_framebuffer, 17);
    if (rst != HAL_OK) {
        return AsyncResult::fromResult((Result)rst);
    }
    return _asyncSource.getResult();
}

void DShot::onCplt(TIM_HandleTypeDef* tim) {
    DShot* dshot = static_cast<DShot*>(PeripheralManager::getInstance().getPeripheral(tim));
    dshot->_asyncSource.setDone();
};

void DShot::onError(TIM_HandleTypeDef* tim) {
    DShot* dshot = static_cast<DShot*>(PeripheralManager::getInstance().getPeripheral(tim));

    dshot->_asyncSource.setError(Result::kError);
};

}  // namespace wibot

#endif  // HAL_TIM_MODULE_ENABLED
