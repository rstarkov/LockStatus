#include "stdafx.h"

// http://stackoverflow.com/questions/2938966/how-to-use-vc-intrinsic-functions-w-o-run-time-library/2945619#2945619
#pragma function(memset)
void * __cdecl memset(void *pTarget, int value, size_t cbTarget) {
    unsigned char *p = static_cast<unsigned char *>(pTarget);
    while (cbTarget-- > 0) {
        *p++ = static_cast<unsigned char>(value);
    }
    return pTarget;
}
