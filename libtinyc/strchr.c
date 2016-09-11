#include "string.h"

char *
strchr (const char *s, int c)
{
  char thechar;
  do
    {
      thechar = *s;
      if (thechar == c)
        return (char *) s;
      s++;
    }
  while (thechar != '\0');
  
  return NULL;
}
