#include "stdio.h"

int fputs (const char *str, FILE *f)
{
  while (*str)
    {
      fputc (*str, f);
      str++;
    }

  return 1;
}
