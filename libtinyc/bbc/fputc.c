#include "stdio.h"

int fputc (int c, FILE *f)
{
  register char x asm ("a") = c;
  if (x == '\n')
    /* OSNEWL */
    __asm__ __volatile__ ("jsr $ffe7");
  else
    /* OSWRCH */
    __asm__ __volatile__ ("jsr $ffee" : : "Aq" (x));

  return c;
}
