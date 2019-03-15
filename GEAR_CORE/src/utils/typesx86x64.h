#pragma once
#include "gear_common.h"

#ifdef _M_X64
typedef int64_t INT;
typedef uint64_t UINT;
#elif _M_IX86
typedef int32_t INT;
typedef uint32_t UINT;
#endif

