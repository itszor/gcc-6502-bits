#include "string.h"

int
memcmp (const void *s1, const void *s2, size_t n)
{
  const unsigned char *c1 = s1, *c2 = s2;
  unsigned int i;
  
  for (i = 0; i < n; i++)
    {
      int diff = c1[i] - c2[i];
      if (diff != 0)
        return diff;
    }
  
  return 0;
}
