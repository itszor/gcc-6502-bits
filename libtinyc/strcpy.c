#include "string.h"

char *
strcpy (char *dest, const char *src)
{
  char *odest = dest;

  while ((*dest++ = *src++))
    ;

  return odest;
}
