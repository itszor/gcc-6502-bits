#include "string.h"

void *
memmove (void *dest, const void *src, size_t n)
{
  unsigned char *d = dest;
  unsigned const char *s = src;
  int i;

  if (dest < src)
    {
      for (i = 0; i < n; i++)
        d[i] = s[i];
    }
  else
    {
      for (i = n - 1; i >= 0; i--)
        d[i] = s[i];
    }
  
  return dest;
}
