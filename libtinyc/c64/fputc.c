#include "stdio.h"

int fputc (int c, FILE *f)
{
  register char x asm ("a") = c;
  __asm__ __volatile__ ("jsr $ffd2" : : "Aq" (x));

  return c;
}
