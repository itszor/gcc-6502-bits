#include <stdarg.h>

#include "stdio.h"
#include "string.h"

char __m65x_char_to_file = 1;

static void
emit_char (int c, FILE *f)
{
  if (__m65x_char_to_file)
    fputc (c, f);
  else
    {
      char **ptr = (char **) f;
      **ptr = c;
      (*ptr)++;
    }
}

/* A version of fputs that knows about the __m65x_char_to_file hack.  */
static void
sfputs (const char *str, FILE *f)
{
  const char *iter;
  for (iter = str; *iter; iter++)
    emit_char (*iter, f);
}

#define M65X_FLOAT_PRINT

static int print_udec (FILE *f, unsigned long val)
{
  char digits[10];
  int c = 0;
  int printed = 0;
  
  do
    {
      digits[c++] = val % 10;
      val = val / 10;
    }
  while (val > 0);

  for (--c; c >= 0; c--)
    {
      emit_char (digits[c] + '0', f);
      printed++;
    }

  return printed;
}

static int print_sdec (FILE *f, long val)
{
  int printed = 0;
  if (val < 0)
    {
      emit_char ('-', f);
      printed += print_udec (f, -val) + 1;
    }
  else
    printed += print_udec (f, val);

  return printed;
}

static int print_hex (FILE *f, unsigned long val)
{
  unsigned char seen_nonzero = 0;
  int printed = 0;
  int i;
  
  for (i = 7; i >= 0; i--)
    {
      unsigned char nybble = (val >> 28) & 0xf;
      
      if (nybble > 0 || i == 0 || seen_nonzero)
        {
	  emit_char (nybble < 10 ? nybble + '0' : nybble - 10 + 'a', f);
          printed++;
        }
      
      if (nybble != 0)
        seen_nonzero = 1;

      val <<= 4;
    }

  return printed;
}

#ifdef M65X_FLOAT_PRINT
extern signed char __m65x_ftoa (char *, float);

static int
print_float (FILE *f, float x)
{
  signed char exp;
  int printed = 0;
  char output[16];
  exp = __m65x_ftoa (&output[0], x);
  sfputs (output, f);
  printed = strlen (output);
  if (exp != 0)
    printed += fprintf (f, "E%d", exp);
  return printed;
}
#endif

#ifndef MAIN
int vfprintf (FILE *f, const char *fmt, va_list ap)
{
  int printed = 0;

  while (*fmt)
    {
      if (*fmt == '%')
        {
	retry:
	  switch (*++fmt)
	    {
            case 'c':
              {
                int val = va_arg (ap, int);
                emit_char (val, f);
                printed++;
              }
              break;

	    case 'l':
	      {
	        switch (*++fmt)
		  {
		  case 'd':
		    {
		      long val = va_arg (ap, long);
		      printed += print_sdec (f, val);
		    }
		    break;

		  case 'u':
		    {
		      unsigned long val = va_arg (ap, unsigned long);
		      printed += print_udec (f, val);
		    }
		    break;

		  case 'x':
		    {
		      unsigned long val = va_arg (ap, unsigned long);
		      printed += print_hex (f, val);
		    }
		    break;

		  default:
		    ;
		  }
	      }
	      break;

	    case 'd':
	      {
	        int val = va_arg (ap, int);
		printed += print_sdec (f, val);
	      }
	      break;
	      
	    case 'u':
	      {
	        unsigned int val = va_arg (ap, unsigned int);
		printed += print_udec (f, val);
	      }
	      break;

	    case 's':
	      {
		char *str = va_arg (ap, char *);
		sfputs (str, f);
                printed += strlen (str);
	      }
	      break;

	    case 'p':
	    case 'x':
	      {
	        unsigned int val = va_arg (ap, unsigned int);
		printed += print_hex (f, val);
	      }
	      break;

	    case '%':
	      emit_char ('%', f);
              printed++;
	      break;

#ifdef M65X_FLOAT_PRINT
	    case 'f':
	      {
		float val = va_arg (ap, double);
		printed += print_float (f, val);
	      }
	      break;
#endif
	    default:
	      /* Just skip unknown % formatting codes.  */
	      goto retry;
	    }
	}
      else
        {
          emit_char (*fmt, f);
          printed++;
        }

      fmt++;
    }

  /* Zero-terminate if printing to string.  */
  if (!__m65x_char_to_file)
    emit_char (0, f);

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
