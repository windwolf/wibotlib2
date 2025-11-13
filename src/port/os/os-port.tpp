#pragma once

#if defined(NORTOS)
#include "nortos/os-port.tpp"
#endif

#if defined(FREERTOS)
#include "freertos/os-port.tpp"
#endif

#if defined(THREADX)
#include "threadx/os-port.tpp"
#endif
