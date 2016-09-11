#include "string.h"

char *
strrchr (const char *s, int c)
{
  char *last = NULL;
  char thechar;

  do
    {
      thechar = *s;
      if (thechar == c)
        last = (char *) s;
      s++;
    }
  while (thechar != '\0');
  
  return last;
}
