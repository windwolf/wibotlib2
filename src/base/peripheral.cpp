
#include "peripheral.hpp"

namespace wibot {

PeripheralManager::PeripheralManager() {};

PeripheralManager &PeripheralManager::getInstance() {
    static PeripheralManager instance;
    return instance;
}

void PeripheralManager::registerPeripheral(PeripheralBase *peripheral, void *instance) {
    if (_head == nullptr) {
        _head                 = peripheral;
        peripheral->_next     = nullptr;
        peripheral->_instance = instance;
    } else {
        _head->append(peripheral);
        peripheral->_instance = instance;
    }
}

void PeripheralManager::unregisterPeripheral(PeripheralBase *peripheral) {
    if (_head == nullptr) {
        return;
    }
    if (_head == peripheral) {
        _head                 = static_cast<PeripheralBase *>(peripheral->_next);
        peripheral->_next     = nullptr;
        peripheral->_instance = nullptr;
    } else {
        _head->remove(peripheral);
    }
}

PeripheralBase *PeripheralManager::getPeripheral(void *instance) {
    if (_head == nullptr) {
        return nullptr;
    }

    auto peri = _head;

    while (peri != nullptr && peri->_instance != instance) {
        peri = static_cast<PeripheralBase *>(peri->_next);
    }

    return peri;
}

PeripheralBase::PeripheralBase() : _instance(nullptr) {
}

// LLPeripheralManager &LLPeripheralManager::getInstance() {
//     static LLPeripheralManager instance;
//     return instance;
// }

// LLPeripheralManager::LLPeripheralManager() : _head(nullptr) {
// }

// void LLPeripheralManager::registerLLPeripheral(u32 scope, void *perp, u32 channel,
//                                                void *instance) {
//     LLPeripheralEntry *entry = new LLPeripheralEntry(perp, instance);
//     entry->scope             = scope;
//     entry->channel           = channel;

//     if (_head == nullptr) {
//         _head        = entry;
//         entry->_next = nullptr;
//     } else {
//         _head->append(entry);
//     }
// }
}  // namespace wibot
