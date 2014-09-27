#include "string.h"

void *
memcpy (void *dest, const void *src, size_t n)
{
  unsigned char *d = dest;
  const unsigned char *s = src;
  unsigned int i;
  
  for (i = 0; i < n; i++)
    d[i] = s[i];
  
  return dest;
}
