#pragma once

#include "async.hpp"
#include "chip.hpp"
#include "analog-source.hpp"
#include "system.hpp"
#include "peripheral.hpp"
#include <cstring>

#ifdef HAL_COMP_MODULE_ENABLED
namespace wibot {

enum class ComparerLevel {
    kLow  = 0,
    kHigh = 1,
};

class Comparer : private PeripheralBase {
   public:
    Comparer(COMP_HandleTypeDef& ins);

    AsyncResult start();
    Result      stop();

    ComparerLevel getLevel() const;

   private:
    static void _onCompareTrigger(COMP_HandleTypeDef* hcomp);

   private:
    COMP_HandleTypeDef* _ins;
    AsyncSource         _compareEventSource;
};

}  // namespace wibot

#endif
