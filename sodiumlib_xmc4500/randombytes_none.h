
#ifndef randombytes_none_H
#define randombytes_none_H

#include <sodium/export.h>
#include <sodium/randombytes.h>

#ifdef __cplusplus
extern "C" {
#endif

SODIUM_EXPORT
extern struct randombytes_implementation randombytes_none_implementation;

#ifdef __cplusplus
}
#endif

#endif
