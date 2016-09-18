#include "stdio.h"

size_t
fwrite (const void *ptr, size_t size, size_t nmemb, FILE *f)
{
  size_t memct, sizect;
  const char *ptrc = ptr;

  for (memct = 0; memct < nmemb; memct++)
    for (sizect = 0; sizect < size; sizect++)
      {
	fputc (*ptrc, f);
	ptrc++;
      }

  return memct;
}
