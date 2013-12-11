/* A tiny C library.  */

__attribute__((noreturn)) void
exit (int status)
{
  __asm__ __volatile__ ("jmp 0");
  while (1)
    ;
}

