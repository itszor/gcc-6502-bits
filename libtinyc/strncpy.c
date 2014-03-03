#include "string.h"

char *
strncpy (char *dest, const char *src, size_t n)
{
  unsigned int i;
  
  for (i = 0; i < n && *src; i++, dest++, src++)
    *dest = *src;
  
  return dest;
}
