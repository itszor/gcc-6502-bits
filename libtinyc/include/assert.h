#ifndef _ASSERT_H
#define _ASSERT_H 1

#include <stdio.h>
#include <stdlib.h>

#ifndef NDEBUG

#define _STRINGIFY(N) _STRINGIFY2(N)
#define _STRINGIFY2(N) #N

#define assert(N)                                                             \
  do                                                                          \
    if (!(N))                                                                 \
      {                                                                       \
        fputs (__FILE__ ":" _STRINGIFY(__LINE__) ": Assertion failed: '"      \
               #N "'\n", stderr);                                             \
        abort ();                                                             \
      }                                                                       \
  while (0)

#else

#define assert(N)

#endif

#endif
