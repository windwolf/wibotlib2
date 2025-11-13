#pragma once

#include "type.hpp"

namespace wibot {

/**
 * For building a map from HAL instance to Peripheral instance.
 */
class PeripheralBase : protected LinkList {
   public:
    PeripheralBase();

   private:
    friend class PeripheralManager;
    void* _instance;
};

class PeripheralManager {
   public:
    static PeripheralManager& getInstance();

   public:
    PeripheralManager();
    PeripheralManager(const PeripheralManager&)            = delete;
    PeripheralManager(const PeripheralManager&&)           = delete;
    PeripheralManager& operator=(const PeripheralManager&) = delete;

    void            registerPeripheral(PeripheralBase* peripheral, void* instance);
    void            unregisterPeripheral(PeripheralBase* peripheral);
    PeripheralBase* getPeripheral(void* instance);

   private:
    PeripheralBase* _head;
};

struct LLPeripheralEntry : protected LinkList {
   public:
    LLPeripheralEntry(void* peripheral, u32 instance) {
    }

    friend class LLPeripheralManager;
    u32   scope;
    void* perp;
    u32   channel;
    void* instance;
};

class LLPeripheralManager {
   public:
    static LLPeripheralManager& getInstance();

   public:
    LLPeripheralManager();
    LLPeripheralManager(const LLPeripheralManager&)            = delete;
    LLPeripheralManager(const LLPeripheralManager&&)           = delete;
    LLPeripheralManager& operator=(const LLPeripheralManager&) = delete;

    void               registerLLPeripheral(u32 scope, void* perp, u32 channel, void* instance);
    void               unregisterLLPeripheral(void* instance);
    LLPeripheralEntry* getLLPeripheral(u32 scope, void* perp, u32 channel);

   private:
    LLPeripheralEntry* _head;
};

}  // namespace wibot
