#if defined(__gba__)
#include "../gba/platform.h"
#elif defined(__integrator__)
#include "../integrator/platform.h"
#elif defined(__spmp__)
#include "../spmp/platform.h"
#else
#error platform not supported
#endif
