#include <stdarg.h>

#include "stdio.h"

static void print_sdec (FILE *f, int val)
{
}

static void print_udec (FILE *f, unsigned int val)
{
}

static void print_hex (FILE *f, unsigned int val)
{
  unsigned char seen_nonzero = 0;
  int i;
  
  for (i = 3; i >= 0; i--)
    {
      unsigned char nybble = (val >> 12) & 0xf;
      
      if (nybble > 0 || i == 0 || seen_nonzero)
	fputc (nybble < 10 ? nybble + '0' : nybble - 10 + 'a', f);
      
      if (nybble != 0)
        seen_nonzero = 1;

      val <<= 4;
    }
}

#ifndef MAIN
int vfprintf (FILE *f, const char *fmt, va_list ap)
{
  int printed = 0;

  while (*fmt)
    {
      if (*fmt == '%')
        {
	  switch (*++fmt)
	    {
	    case 'd':
	      {
	        int val = va_arg (ap, int);
		print_sdec (f, val);
	      }
	      break;
	      
	    case 'u':
	      {
	        unsigned int val = va_arg (ap, unsigned int);
		print_udec (f, val);
	      }
	      break;

	    case 'x':
	      {
	        int val = va_arg (ap, int);
		print_hex (f, val);
	      }
	      break;
	    
	    default:
	      ;
	    }
	}
      else
        fputc (*fmt, f);

      fmt++;
      printed++;
    }
  
  return printed;
}
#endif

#ifdef MAIN
void *ptr;

__attribute__((noinline)) void
little_vararg (int* unk, ...)
{
  va_list ap;
  va_start (ap, unk);
  //ptr = (void*) ap;
  print_hex (stdout, va_arg (ap, int));
  va_end (ap);
}

int main (int argc, char* argv[])
{
  int i;
#if 0
  print_hex (stdout, 0xa0);
#else
  for (i = 0; i < 257; i++)
    {
#if 1
      little_vararg (0, i);
#else
      print_hex (stdout, i);
#endif
      fputc ('\n', stdout);
    }
#endif
}
#endif
