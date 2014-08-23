#include "string.h"

char *
strncpy (char *dest, const char *src, size_t n)
{
  unsigned int i;
  char *odest = dest;
  
  for (i = 0; i < n && *src; i++, dest++, src++)
    *dest = *src;
  
  return odest;
}
