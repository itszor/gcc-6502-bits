#include "string.h"

size_t
strlen (const char *s)
{
  unsigned int count = 0;
  
  while (*s++)
    count++;
  
  return count;
}
