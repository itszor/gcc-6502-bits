#include "stdlib.h"

static unsigned long state = 1;

void
srand (unsigned int newstate)
{
  state = (unsigned long) newstate + 17;
}

/* This is a basic implementation of xorshift32.  The internal state is 32
   bits, but we're only returning the low-order 15 of those.  */

int
rand (void)
{
  unsigned long lstate = state;
  lstate ^= lstate << 13;
  lstate ^= lstate >> 17;
  lstate ^= lstate << 5;
  state = lstate;
  return state & 0x7fff;
}
