#pragma once

#include "targetver.h"
#include "resource.h"

#include <Windows.h>
#include <strsafe.h>

// http://stackoverflow.com/questions/2938966/how-to-use-vc-intrinsic-functions-w-o-run-time-library/2945619#2945619
extern "C" void * __cdecl memset(void *, int, size_t);
#pragma intrinsic(memset)
