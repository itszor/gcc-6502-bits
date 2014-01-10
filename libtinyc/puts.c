#include "stdio.h"

int puts (const char *str)
{
  fputs (str, stdout);
  fputc ('\n', stdout);
  return 1;
}
