#if defined (NORTOS)
#include "nortos/os-port.cpp"
#endif


#if defined (FREERTOS)
#include "freertos/os-port.cpp"
#endif

#if defined (THREADX)
#include "threadx/os-port.cpp"
#endif