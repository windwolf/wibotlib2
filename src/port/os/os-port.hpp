#pragma once

#if defined(NORTOS)
#include "nortos/os-port.hpp"
#endif

#if defined(FREERTOS)
#include "freertos/os-port.hpp"
#endif

#if defined(THREADX)
#include "threadx/os-port.hpp"
#endif
