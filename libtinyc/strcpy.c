#include "string.h"

char *
strcpy (char *dest, const char *src)
{
  while ((*dest++ = *src++))
    ;
}
