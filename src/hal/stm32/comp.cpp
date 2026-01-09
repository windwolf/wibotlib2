#include "comp.hpp"
#include "stm32g4xx_hal_comp.h"
#ifdef HAL_COMP_MODULE_ENABLED
namespace wibot::hal {

void Comparer::_onCompareTrigger(COMP_HandleTypeDef* hcomp) {
    auto instance = (Comparer*)PeripheralManager::getInstance().getPeripheral(hcomp);
    instance->_compareEventSource.setDone();
}

Comparer::Comparer(COMP_HandleTypeDef& ins) : _ins(&ins), _compareEventSource() {
    HAL_COMP_RegisterCallback(&ins, HAL_COMP_TRIGGER_CB_ID, _onCompareTrigger);
    PeripheralManager::getInstance().registerPeripheral(this, _ins);
}

os::AsyncResult Comparer::start() {
    auto rst = HAL_COMP_Start(_ins);
    if (rst != HAL_OK) {
        return os::AsyncResult::fromError(Result::kError);
    }
    return _compareEventSource.getResult(true);
};

Result Comparer::stop() {
    auto rst = HAL_COMP_Stop(_ins);
    return Result(rst);
};

ComparerLevel Comparer::getLevel() const {
    uint32_t level = HAL_COMP_GetOutputLevel(_ins);
    return static_cast<ComparerLevel>(level);
};
}  // namespace wibot::hal
#endif
