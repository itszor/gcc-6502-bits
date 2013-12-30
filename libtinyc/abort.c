
__attribute__((noreturn)) void
abort (void)
{
  register unsigned char retcode __asm__ ("a") = 1;
  __asm__ __volatile__ ("jmp 0" : : "Aq" (retcode));
  while (1)
    ;
}

