#include "chip.hpp"

#if defined(STM32F1xx)
#include "stm32f1/chip.cpp"

#endif

#if defined(STM32G4xx)
#include "stm32g4/chip.cpp"
#endif

#if defined(STM32G0xx)
#include "stm32g0/chip.cpp"

#endif

#if defined(STM32H7xx)
#include "stm32h7/chip.cpp"

#endif
