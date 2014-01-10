#include <stdarg.h>

#include "stdio.h"

int fprintf (FILE *f, const char *fmt, ...)
{
  va_list ap;
  int retcode;
  
  va_start (ap, fmt);
  retcode = vfprintf (f, fmt, ap);
  va_end (ap);
  
  return retcode;
}
