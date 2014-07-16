#include "string.h"

char *
strchr (const char *s, int c)
{
  for (; *s != NULL; s++)
    {
      if (*s == c)
        return s;
    }
  
  return NULL;
}
