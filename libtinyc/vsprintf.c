#include <stdarg.h>

#include "stdio.h"

int
vsprintf (char *str, const char *fmt, va_list ap)
{
  int rc;
  
  __m65x_char_to_file = 0;
  rc = vfprintf ((FILE *) &str, fmt, ap);
  __m65x_char_to_file = 1;

  return rc;
}
