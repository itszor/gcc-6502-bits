#include <stdarg.h>

#include "stdio.h"

int
sprintf (char *str, const char *fmt, ...)
{
  va_list ap;
  int rc;
  
  __m65x_char_to_file = 0;
  va_start (ap, fmt);
  rc = vfprintf ((FILE *) &str, fmt, ap);
  va_end (ap);
  __m65x_char_to_file = 1;

  return rc;
}
