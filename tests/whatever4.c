int friendly_global = 1;
int another_global = 2;
int third_global;
int fourth_global;

static void __attribute__ ((used))
foo (unsigned int *arr, int i)
{
  fourth_global = third_global;
  third_global = friendly_global + another_global;
}
