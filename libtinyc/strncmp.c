#include "string.h"

int
strncmp (const char *s1, const char *s2, size_t n)
{
  unsigned int i;

  for (i = 0; i < n && (*s1 || *s2); i++, s1++, s2++)
    {
      int diff;
      
      if (!*s1 || !*s2)
	return *(unsigned char *)s1 - *(unsigned const char*)s2;
      
      diff = *s1 - *s2;
      if (diff != 0)
        return diff;
    }
  
  return 0;
}
