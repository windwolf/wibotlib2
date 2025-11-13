#pragma once

//
// Created by zhouj on 2023/8/30.
//

#include "rtc.hpp"
#include "peripheral.hpp"
#

#ifdef HAL_RTC_MODULE_ENABLED

namespace wibot {

class HalRtc : public Rtc, private PeripheralBase, private Initializable {
   public:
    HalRtc(RTC_HandleTypeDef *handle);
    ~HalRtc();

    Result read(DateTime &datetime) override;
    Result write(DateTime datetime) override;

   private:
    void _init() override;

   private:
    RTC_HandleTypeDef *_handle;
};

}  // namespace wibot

#endif
