#include "string.h"

char *
strncpy (char *dest, const char *src, size_t n)
{
  unsigned int i;
  char *odest = dest;

  for (i = 0; i < n; dest++, src++, i++)
    {
      char c = *dest = *src;
      if (!c)
	break;
    }

  /* Pad with NUL chars.  */
  for (; i < n; dest++, i++)
    *dest = '\0';

  return odest;
}
