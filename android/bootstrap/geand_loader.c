#define _GNU_SOURCE
#include "geand_loader.h"

#include <android/dlext.h>
#include <dlfcn.h>
#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>

#define GEAND_CORE_BASE ((uintptr_t)0x20000000u)
#define GEAND_CORE_RESERVE ((size_t)0x10000000u) /* 256 MiB */

static char g_loader_error[512];

static void geand_set_error(const char *what)
{
    const char *dl = dlerror();
    if (dl && *dl) {
        snprintf(g_loader_error, sizeof(g_loader_error), "%s: %s", what, dl);
    } else {
        snprintf(g_loader_error, sizeof(g_loader_error), "%s: errno=%d (%s)",
                 what, errno, strerror(errno));
    }
}

const char *geand_loader_error(void)
{
    return g_loader_error;
}

void *geand_load_core_low(const char *absolute_library_path)
{
    void *reserve;
    void *handle;
    android_dlextinfo ext;

    g_loader_error[0] = 0;

    if (!absolute_library_path || !*absolute_library_path) {
        snprintf(g_loader_error, sizeof(g_loader_error),
                 "libge007 path is empty");
        return NULL;
    }

    /*
     * A non-MAP_FIXED exact hint is deliberately used first. It cannot
     * overwrite an existing mapping. If the requested low range is occupied,
     * Android may choose another address; reject that result and fail cleanly.
     */
    reserve = mmap((void *)GEAND_CORE_BASE, GEAND_CORE_RESERVE,
                   PROT_NONE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (reserve == MAP_FAILED) {
        geand_set_error("low-address reservation mmap failed");
        return NULL;
    }

    if ((uintptr_t)reserve != GEAND_CORE_BASE) {
        munmap(reserve, GEAND_CORE_RESERVE);
        snprintf(g_loader_error, sizeof(g_loader_error),
                 "low-address range unavailable: wanted %p, got %p",
                 (void *)GEAND_CORE_BASE, reserve);
        return NULL;
    }

    memset(&ext, 0, sizeof(ext));
    ext.flags = ANDROID_DLEXT_RESERVED_ADDRESS;
    ext.reserved_addr = reserve;
    ext.reserved_size = GEAND_CORE_RESERVE;

    dlerror();
    handle = android_dlopen_ext(absolute_library_path, RTLD_NOW | RTLD_LOCAL, &ext);
    if (!handle) {
        geand_set_error("android_dlopen_ext(libge007) failed");
        munmap(reserve, GEAND_CORE_RESERVE);
        return NULL;
    }

    /*
     * The reserved mapping is consumed/replaced by the loader as needed.
     * Do not munmap it after a successful load; the loader owns mappings
     * inside this address range for the process lifetime.
     */
    return handle;
}
