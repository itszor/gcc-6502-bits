int friendly_global = 1;
int another_global[3];

int
foo (unsigned int *arr, int i)
{
  return another_global[2] + friendly_global + arr[2];
}
