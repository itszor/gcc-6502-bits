#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>

/***

   31    24 23    16 15     8 7      0
  .--------.--------.--------.--------.
  |exponent|s| mant |  mant  |  mant  |
  '--------'--------'--------'--------'

***/

typedef uint32_t sftype;

static int16_t s_exp;
static uint32_t s_mant;
static char s_sign;

static void
unpack (sftype x)
{
  s_exp = x >> 24;
  s_sign = (x & 0x800000) != 0;
  s_mant = x & 0x7fffff;
  
  if (s_exp != 0)
    s_mant |= 0x800000;
}

static sftype
repack (uint32_t mant, uint16_t exp, char sign)
{
  return (mant & 0x7fffff) | ((sign & 1) << 23) | ((exp & 0xff) << 24);
}

typedef union
{
  float f;
  uint32_t i;
} u;

sftype float_to_sftype (float x)
{
  u tmp;
  tmp.f = x;
  return ((tmp.i & 0x80000000u) >> 8)
	 | ((tmp.i & 0x7f800000u) << 1)
	 | (tmp.i & 0x007fffffu);
}

static void
denormalize (void)
{
  if (s_exp > 127 && s_exp <= 130)
    {
      int shift = s_exp - 127;
      s_mant <<= shift;
      s_exp -= shift;
    }
}

static void
print_unpacked (const char *routine)
{
  return;
  printf ("-- %s --\n", routine);
  printf ("mantissa: %x\n", s_mant);
  printf ("exponent: %d\n", s_exp);
  printf ("sign: %d\n", s_sign);
}

static void
mul10 (void)
{
  print_unpacked ("mul");

  s_mant *= 10;

  while ((s_mant & 0xff000000) != 0)
    {
      s_mant >>= 1;
      s_exp++;
    }
}

static void
div10 (void)
{
  print_unpacked ("div");

#if 0
  s_mant <<= 8;
  s_mant /= 10u;
  
  while ((s_mant & 0x80000000) == 0)
    {
      s_mant <<= 1;
      s_exp--;
    }
  
  s_mant = (s_mant + 128) >> 8;
#else
  s_exp -= 4;
  
  s_mant += s_mant / 2;
  s_mant += s_mant / 16;
  s_mant += s_mant / 256;
  s_mant += s_mant / 65536;
  
  while ((s_mant & 0xff000000) != 0)
    {
      s_mant >>= 1;
      s_exp++;
    }
#endif
}

sftype
fp_add (sftype a, sftype b)
{
  bool a_sign = (a & 0x800000) != 0;
  bool b_sign = (b & 0x800000) != 0;
  int16_t a_exp = a >> 24;
  int16_t b_exp = b >> 24;
  uint32_t a_mant = a & 0x7fffff;
  uint32_t b_mant = b & 0x7fffff;
  
  if (a_exp != 0)
    a_mant |= 0x800000;

  if (b_exp != 0)
    b_mant |= 0x800000;
  
  if (a_sign != b_sign)
    abort ();
  
  if (a_exp > b_exp)
    {
      if (a_exp > b_exp + 23)
        return a;

      a_mant += b_mant >> (a_exp - b_exp);

      while ((a_mant & 0xff000000) != 0)
        {
	  a_mant >>= 1;
	  a_exp++;
	}

      return repack (a_mant, a_exp, a_sign);
    }
  else
    {
      if (b_exp > a_exp + 23)
        return b;
      
      b_mant += a_mant >> (b_exp - a_exp);
      
      while ((b_mant & 0xff000000) != 0)
        {
	  b_mant >>= 1;
	  b_exp++;
	}

      return repack (b_mant, b_exp, b_sign);
    }
}

void
fp_print (FILE *f, sftype x)
{
  int dec_exp, place;
  bool print_exp = false;
  int i, sigfigs = 7;
  sftype bias = float_to_sftype (5.0);
  
  unpack (bias);
  for (i = 0; i < sigfigs; i++)
    div10 ();
  bias = repack (s_mant, s_exp, s_sign);
  
  unpack (x);
  dec_exp = 0;
  
  print_unpacked ("start");
  
  while (s_exp < 127)
    {
      mul10 ();
      dec_exp--;
    }
  
  while (s_exp > 130 || (s_exp == 130 && s_mant >= 0xa00000))
    {
      div10 ();
      dec_exp++;
    }
  
  x = fp_add (repack (s_mant, s_exp, s_sign), bias);
  unpack (x);
  
  if (dec_exp < -2 || dec_exp > 6)
    print_exp = true;
  
  if (dec_exp < 0)
    {
      fputs ("0.", f);
      for (place = dec_exp; place < -1; place++)
	fputc ('0', f);
    }
  
  for (place = 0;
       (place < sigfigs && s_mant != 0)
       || (!print_exp && place <= dec_exp);
       place++)
    {
      char digit;
      bool lastplace = (place == sigfigs - 1);
            
      denormalize ();
      
      digit = s_mant >> 23;
      
      fputc (digit + '0', f);

      s_mant = s_mant - (digit << 23);

      if ((print_exp && place == 0 && !lastplace)
	  || (!print_exp && place == dec_exp && !lastplace))
	fputc ('.', f);
      
      mul10 ();
    }
  
  if (print_exp)
    {
      fputc ('E', f);
      fprintf (f, "%d", dec_exp);
    }
}


int main (void)
{
  float q;
  int i;
  
  fp_print (stdout, float_to_sftype (1.0));
  fputc ('\n', stdout);
  
  fp_print (stdout, float_to_sftype (9.9999));
  fputc ('\n', stdout);

  fp_print (stdout, float_to_sftype (10));
  fputc ('\n', stdout);

  fp_print (stdout, float_to_sftype (100));
  fputc ('\n', stdout);

  fp_print (stdout, float_to_sftype (135));
  fputc ('\n', stdout);

  fp_print (stdout, float_to_sftype (0.1));
  fputc ('\n', stdout);

  fp_print (stdout, float_to_sftype (1234.56));
  fputc ('\n', stdout);

  fp_print (stdout, float_to_sftype (1000000.5));
  fputc ('\n', stdout);

  fp_print (stdout, float_to_sftype (23400000.5));
  fputc ('\n', stdout);

  for (i = 0; i < 30; i++)
    {
      fp_print (stdout, float_to_sftype ((float) 1.5 + i / 10.0));
      fputc ('\n', stdout);
    }
  
  for (i = 0; i < 30; i++)
    {
      float rnd = drand48 () * 1000.0;
      printf ("num: %f\nmy routine: ", rnd);
      fp_print (stdout, float_to_sftype (rnd));
      fputc ('\n', stdout);
    }
}
