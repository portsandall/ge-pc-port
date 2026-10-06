#include <android/api-level.h>
#include <stdint.h>
#include <stddef.h>

#if !defined(__ANDROID__)
#error GEAND platform probe must be compiled by the Android NDK
#endif

#if !defined(__aarch64__)
#error GEAND currently targets arm64-v8a
#endif

_Static_assert(sizeof(void *) == 8, "GEAND requires a 64-bit Android ABI");
_Static_assert(sizeof(uintptr_t) == 8, "uintptr_t must be 64 bit");

int geand_android_platform_probe(void)
{
    return __ANDROID_API__;
}
