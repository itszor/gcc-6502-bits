#include <stdarg.h>

#include "stdio.h"

int printf (const char *fmt, ...)
{
  va_list ap;
  int retcode;
  
  va_start (ap, fmt);
  retcode = vfprintf (stdout, fmt, ap);
  va_end (ap);
  
  return retcode;
}
