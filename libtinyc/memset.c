#include "string.h"

void *
memset (void *s, int c, size_t n)
{
  unsigned char cbyte = c;
  unsigned char *scp = s;
  unsigned int nint = n;
  
  __asm__ __volatile__ 
    (
    "ldy #0\n\t"
    "ldx %2+1\n\t"
    "beq :++\n\t"
    ":\n\t"
    "sta (%0),y\n\t"
    "iny\n\t"
    "bne :-\n\t"
    "inc %0+1\n\t"
    "dex\n\t"
    "bne :-\n\t"
    ":\n\t"
    "ldy %2\n\t"
    "beq :++\n\t"
    ":\n\t"
    "dey\n\t"
    "sta (%0),y\n\t"
    "bne :-\n\t"
    ":\n\t"
    :
    : "Zp" (scp), "Aq" (cbyte), "Zp" (nint)
    : "x", "y");

  return s;
}
