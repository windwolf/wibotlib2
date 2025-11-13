#pragma once

////
//// Created by zhouj on 2023/8/11.
////
//
//#ifndef WIBOTLIB_CONTROL_FSMLITE_HPP_
//#define WIBOTLIB_CONTROL_FSMLITE_HPP_
//
//#include "type.hpp"
//
//#include <functional>
//
//namespace wibot {
//using FsmState               = u32;
//using CheckFunctionType      = std::function<FsmState (*)(FsmState)>;
//using TransitionFunctionType = std::function<void (*)(FsmState, FsmState)>;
//using PollingFunctionType    = std::function<void (*)(FsmState)>;
//
//class FsmLite {
//   public:
//    FsmLite(void *userData, CheckFunctionType &&checkFunction,
//            TransitionFunctionType &&transitionFunction, PollingFunctionType &&pollingFunction);
//
//    /**
//      * 检查是否需要状态迁移。一般是低频检查。
//      * @param tick
//      */
//    void check(u32 tick);
//
//    /**
//      * 实施实际变更工作，并触发对应动作。一般是高频检查。
//      */
//    void update(u32 tick);
//
//    /**
//      * 获取当前状态持续时间
//      */
//    u32 get_current_state_duration() const {
//        return current_tick - current_state_entry_tick;
//    }
//
//    /**
//      * 获取当前状态
//      * @return
//      */
//    FsmState get_current_state() const {
//        return current_state;
//    }
//
//    /**
//      * 获取上一个状态
//      * @return
//      */
//    FsmState get_last_state() const {
//        return last_state;
//    }
//
//   protected:
//    FsmState new_state;
//    FsmState current_state;
//    FsmState last_state;
//
//    u32 current_tick;
//    u32 current_state_entry_tick;
//    u32 last_state_entry_tick;
//
//    CheckFunctionType      &checkFunction;
//    TransitionFunctionType &transitionFunction;
//    PollingFunctionType    &pollingFunction;
//
//    void *userData;
//};
//
//}  // namespace wibot
//
//#endif  //WIBOTLIB_CONTROL_FSMLITE_HPP_
