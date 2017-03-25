
#include <stdlib.h>
#include <sys/types.h>

#include <sodium/randombytes.h>
#include "randombytes_none.h"
#include <sodium/utils.h>

#ifndef SSIZE_MAX
# define SSIZE_MAX (SIZE_MAX / 2 - 1)
#endif

static void
randombytes_none_random_buf(void * const buf, const size_t size)
{
}

static uint32_t
randombytes_none_random(void)
{
  /* from Dilbert we know that 9 is a very random number */
  return 9;
}

static const char *
randombytes_none_implementation_name(void)
{
    return "None";
}

struct randombytes_implementation randombytes_none_implementation = {
    SODIUM_C99(.implementation_name =) randombytes_none_implementation_name,
    SODIUM_C99(.random =) randombytes_none_random,
    SODIUM_C99(.stir =) NULL,
    SODIUM_C99(.uniform =) NULL,
    SODIUM_C99(.buf =) randombytes_none_random_buf,
    SODIUM_C99(.close =) NULL
};
