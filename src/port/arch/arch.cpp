#include "../port.hpp"

#ifdef CORTEX_M0PLUS
#include "cortex-m0plus/arch.cpp"
#endif

#ifdef CORTEX_M3
#include "cortex-m3/arch.cpp"
#endif

#ifdef CORTEX_M4
#include "cortex-m4/arch.cpp"
#endif

#ifdef CORTEX_M7
#include "cortex-m7/arch.cpp"
#endif