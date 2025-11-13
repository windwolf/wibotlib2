// #include "os.hpp"

// #include "system.hpp"

// #include "arch.hpp"

// namespace wibot {

// void os::sleep(u32 ms) {
//     volatile u32 start = System::getTickMs();
//     while (System::getTickMs() - start < ms) {
//     }
//     return;
// }
// Thread::Thread(const char* name, Slice stack, u32 priority, const ThreadConfig* config)
//     : _name(name), _stack(stack), _priority(priority) {
// }
// Thread::~Thread() {
// }
// void Thread::start(Worker* worker) {
//     run();
// }
// void Thread::_init() {
// }
// void Thread::runStub(u32 instance) {

// };

// Mutex::Mutex(const char* name) : _name(name) {
//     _instance = 0;
// };
// Mutex::~Mutex() {
//     _instance = 0;
// };

// Result Mutex::lock(u32 timeout) {
//     if (timeout == TIMEOUT_NOWAIT) {
//         if (this->_instance) {
//             return Result::kNoResource;
//         } else {
//             this->_instance = 1;
//             return Result::kOk;
//         }
//     } else {
//         if (arch::isIsr()) {
//             return Result::kInvalidOperationFromISR;
//         } else {
//             u32 start = System::getTickMs();
//             while (this->_instance) {
//                 if (System::getDurationMs(start) > timeout) {
//                     return Result::kTimeout;
//                 }
//             };
//             this->_instance = 1;
//             return Result::kOk;
//         }
//     }
// };

// void Mutex::unlock() {
//     this->_instance = 0;
// };

// EventGroup::EventGroup(const char* name) : _name(name) {
//     _instance = 0;
// };
// EventGroup::~EventGroup() {
//     _instance = 0;
// };

// Result EventGroup::set(u32 flags) {
//     auto oldFlags = this->_instance;
//     auto newFlags = this->_instance | flags;
//     while (!wibot::arch::syncCompareAndSwap(&this->_instance, oldFlags, newFlags)) {
//         oldFlags = this->_instance;
//         newFlags = this->_instance | flags;
//     }
//     return Result::kOk;
// };

// Result EventGroup::reset(u32 flags) {
//     auto oldFlags = this->_instance;
//     auto newFlags = this->_instance & ~flags;
//     while (!wibot::arch::syncCompareAndSwap(&this->_instance, oldFlags, newFlags)) {
//         oldFlags = this->_instance;
//         newFlags = this->_instance & ~flags;
//     }
//     return Result::kOk;
// };

// Result EventGroup::wait(u32 flags, u32& actualFlags, EventOptions options, u32 timeout) {
//     Result rst = Result::kOk;

//     if (timeout == TIMEOUT_NOWAIT) {
//         if ((options & EventOptions_WaitFlag) == EventOptions_WaitForAll) {
//             rst = ((this->_instance & flags) == flags) ? Result::kOk : Result::kNoResource;
//         } else {
//             rst = ((this->_instance & flags) != 0) ? Result::kOk : Result::kNoResource;
//         }
//     } else {
//         if (arch::isIsr()) {
//             return Result::kInvalidOperationFromISR;
//         } else {
//             u32 start = System::getTickMs();
//             if ((options & EventOptions_WaitFlag) == EventOptions_WaitForAll) {
//                 while ((this->_instance & flags) != flags) {
//                     if (System::getDurationMs(start) > timeout) {
//                         rst = Result::kTimeout;
//                         break;
//                     }
//                 };
//             } else {
//                 while ((this->_instance & flags) == 0) {
//                     if (System::getDurationMs(start) > timeout) {
//                         rst = Result::kTimeout;
//                         break;
//                     }
//                 };
//             }
//         }
//     }
//     actualFlags = this->_instance;
//     if ((rst == Result::kOk) && ((options & EventOptions_ClearFlag) == EventOptions_ClearFlag)) {
//         reset(flags);
//     }
//     return Result::kOk;
// }
// void EventGroup::_init() {

// };

// MessageQueue::MessageQueue(const char* name, void* msgAddr, u32 msgSize, u32 queueSize)
//     : _instance(static_cast<u8*>(msgAddr), msgSize * sizeof(u32) * queueSize),
//       _name(name),
//       _msgAddr(msgAddr),
//       _msgSize(msgSize),
//       _queueSize(queueSize) {
// }

// MessageQueue::~MessageQueue() {
// }

// Result MessageQueue::send(const void* msg, u32 timeout) {
//     Result rst = Result::kOk;
//     if (!_instance.isFull()) {
//         _instance.write(static_cast<const u8*>(msg), _msgSize * sizeof(u32), false);
//     } else {
//         if (timeout == TIMEOUT_NOWAIT) {
//             return Result::kNoResource;
//         } else {
//             if (arch::isIsr()) {
//                 return Result::kInvalidOperationFromISR;
//             } else {
//                 u32 start = System::getTickMs();
//                 while (_instance.isFull()) {
//                     if (System::getDurationMs(start) > timeout) {
//                         rst = Result::kTimeout;
//                         break;
//                     }
//                 };
//                 if (rst == Result::kOk) {
//                     _instance.write(static_cast<const u8*>(msg), _msgSize * sizeof(u32), false);
//                 }
//             }
//         }
//     }
//     return rst;
// }

// Result MessageQueue::receive(void* msg, u32 timeout) {
//     Result rst = Result::kOk;
//     if (!_instance.isEmpty()) {
//         _instance.read(static_cast<u8*>(msg), _msgSize * sizeof(u32));
//     } else {
//         if (timeout == TIMEOUT_NOWAIT) {
//             return Result::kNoResource;
//         } else {
//             if (arch::isIsr()) {
//                 return Result::kInvalidOperationFromISR;
//             } else {
//                 u32 start = System::getTickMs();
//                 while (_instance.isEmpty()) {
//                     if (System::getDurationMs(start) > timeout) {
//                         rst = Result::kTimeout;
//                         break;
//                     }
//                 };
//                 if (rst == Result::kOk) {
//                     _instance.read(static_cast<u8*>(msg), _msgSize * sizeof(u32));
//                 }
//             }
//         }
//     }
//     return rst;
// }
// Result MessageQueue::flush() {
//     _instance.clear();
//     return Result::kOk;
// }

// }  // namespace wibot
