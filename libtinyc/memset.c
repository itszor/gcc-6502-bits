#include "string.h"

void *
memset (void *s, int c, size_t n)
{
  int i;
  unsigned char cbyte = c;
  unsigned char *scp = s;
  
  for (i = 0; i < n; i++)
    scp[i] = cbyte;
  
  return s;
}
