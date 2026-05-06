#pragma once

#if defined(STM32F103xx)
#define STM32F1xx
#endif

#if defined(STM32G431xx)
#define STM32G4xx
#endif

#if defined(STM32G031xx) || defined(STM32G071xx)
#define STM32G0xx
#endif

#if defined(STM32H750xx)
#define STM32H7xx
#endif

//-------------------

#if defined(STM32F1xx)
#define CORTEX_M3
#endif

#if defined(STM32F4xx) || defined(STM32G4xx)
#define CORTEX_M4
#endif

#if defined(STM32G0xx)
#define CORTEX_M0PLUS
#endif

#if defined(STM32H7xx)
#define CORTEX_M7
#endif
