#ifndef GEAND_LOADER_H
#define GEAND_LOADER_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Load the GoldenEye Android core into a low virtual-address reservation.
 *
 * The existing ARM-GE port intentionally keeps host-visible game pointers
 * below 4 GiB because N64 display lists and several reconstructed code paths
 * still carry addresses through 32-bit fields. Android normally ASLR-loads
 * shared objects high in the process. The bootstrap therefore reserves a
 * low range and asks Bionic's extended loader to place libge007.so inside it.
 *
 * Returns the dlopen handle on success, NULL on failure.
 */
void *geand_load_core_low(const char *absolute_library_path);

/* Human-readable diagnostic for the last loader failure. */
const char *geand_loader_error(void);

#ifdef __cplusplus
}
#endif

#endif
