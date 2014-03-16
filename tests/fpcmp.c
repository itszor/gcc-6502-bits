#include <stdio.h>

float x[] = { 0.0, 1.0, -1.0,  1.0, -1.0, 2.0, 3.0, -2.0, -3.0, -2.0,
              0.0, -0.5 };
float y[] = { 0.0, 1.0, -1.0, -1.0,  1.0, 3.0, 2.0,  3.0, -4.0, -1.0,
              0.5,  0.0 };

int main (void)
{
  int i;

  for (i = 0; i < sizeof(x) / sizeof(x[0]); i++)
    {
      printf ("%d ", x[i] < y[i]);
      printf ("%d ", x[i] <= y[i]);
      printf ("%d ", x[i] > y[i]);
      printf ("%d ", x[i] >= y[i]);
      printf ("%d ", x[i] == y[i]);
      printf ("%d\n", x[i] != y[i]);
    }
  return 0;
}
