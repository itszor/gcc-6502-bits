#include "string.h"

char *
strrchr (const char *s, int c)
{
  char *last = NULL;

  for (; *s != NULL; s++)
    {
      if (*s == c)
        last = s;
    }
  
  return last;
}
