#pragma once

#include "chip.hpp"
#include "peripheral.hpp"
#include "os/async.hpp"

#if defined(HAL_TIM_MODULE_ENABLED)

namespace wibot {

enum class DShotProtocol {
    DShot150 = 0,
    DShot300 = 1,
    DShot600 = 2,
};

class DShot : private PeripheralBase {
   public:
    DShot(TIM_HandleTypeDef& tim, DShotProtocol protocol = DShotProtocol::DShot150);
    ~DShot();

    /**
     * @brief 发送一个DShot命令
     * 
     * @param channel 1-6
     * @param command 
     * @param telemetry 
     * @return AsyncResult 
     */
    AsyncResult send(u8 channel, u16 command, bool telemetry);

   private:
    static void onCplt(TIM_HandleTypeDef* tim);
    static void onError(TIM_HandleTypeDef* tim);

   private:
    TIM_HandleTypeDef& _tim;
    DShotProtocol      _protocol;

    u32                             _pulse0;
    u32                             _pulse1;
    __attribute__((aligned(4))) u16 _framebuffer[17];  // 16个字节的帧缓冲区
    AsyncSource                     _asyncSource;
};

}  // namespace wibot

#endif  // HAL_TIM_MODULE_ENABLED
