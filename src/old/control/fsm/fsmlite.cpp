////
//// Created by zhouj on 2023/8/11.
////
//
//#include "fsmlite.hpp"
//
//namespace wibot {
//void FsmLite::update(u32 tick) {
//    current_tick = tick;
//    if (new_state != current_state) {
//        last_state               = current_state;
//        current_state            = new_state;
//        last_state_entry_tick    = current_state_entry_tick;
//        current_state_entry_tick = current_tick;
//        transitionFunction(last_state, current_state);
//    } else {
//        pollingFunction(current_state);
//    }
//};
//
//void FsmLite::check(u32 tick) {
//    current_tick = tick;
//    new_state    = checkFunction(current_state);
//}
//
//FsmLite::FsmLite(void *userData, CheckFunctionType &&checkFunction,
//                 TransitionFunctionType &&transitionFunction, PollingFunctionType &&pollingFunction)
//    : checkFunction(checkFunction),
//      transitionFunction(transitionFunction),
//      pollingFunction(pollingFunction),
//      userData(userData){
//
//      };
//
//}  // namespace wibot
