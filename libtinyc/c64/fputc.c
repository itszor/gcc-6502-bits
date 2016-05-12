#include "stdio.h"

int fputc (int c, FILE *f)
{
  register char x asm ("a") = c;
  /* FIXME: I don't know how to do this on a C64!  */
  __asm__ __volatile__ ("jsr $ffd2" : : "Aq" (x));

  return c;
}
