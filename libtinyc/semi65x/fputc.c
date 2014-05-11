#include "stdio.h"

int fputc (int c, FILE *f)
{
  volatile char *cp = (char*) 0xfff0;

  if (f == stdout || f == stderr)
    *cp = c;

  return c;
}
