static void
myputc (char c)
{
  volatile char *cp = (char*) 0xfff0;
  *cp = c;
}

static void
myputs (const char *msg)
{
  while (*msg)
    myputc (*msg++);
}

int main (int argc, char* argv)
{
  int i;
  myputs ("Hello world!\n");
  return 0;
}
