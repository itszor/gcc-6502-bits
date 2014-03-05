#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <assert.h>

/***

   31    24 23    16 15     8 7      0
  .--------.--------.--------.--------.
  |exponent|s| mant |  mant  |  mant  |
  '--------'--------'--------'--------'

***/

typedef uint32_t sftype;

static int16_t s_exp;
static uint64_t s_mant;
static char s_sign;

static void
unpack (sftype x)
{
  s_exp = x >> 24;
  s_sign = (x & 0x800000) != 0;
  s_mant = (uint64_t)(x & 0x7fffff) << 8;
  
  if (s_exp != 0)
    s_mant |= 0x80000000;
}

static sftype
repack (uint64_t mant, uint16_t exp, char sign)
{
  mant = mant >> 8;
  assert (mant & 0x800000);
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
  //printf ("denormalize: s_mant=%.8llx, s_exp=%d -> ", s_mant, s_exp);
  while (s_exp < 130)
    {
      s_mant >>= 1;
      s_exp++;
    }
  //printf ("s_mant=%.8llx, s_exp=%d\n", s_mant, s_exp);
}

static void
print_unpacked (const char *routine)
{
  return;
  printf ("%s: ", routine);
  printf ("mantissa: %llx ", s_mant);
  printf ("exponent: %d ", s_exp);
  printf ("sign: %d\n", s_sign);
}

static void
mul10 (void)
{
  s_mant *= 10;
  //s_exp++;

  while ((s_mant & ~0xffffffffull) != 0)
    {
      s_mant >>= 1;
      s_exp++;
    }
  print_unpacked ("mul");
}

static void
div10 (void)
{
  s_exp -= 4;
  
  //printf ("div10 (0): s_mant: %.8llx\n", s_mant);
  
  s_mant += s_mant / 2;
  //printf ("div10 (1): s_mant: %.8llx\n", s_mant);
  s_mant += s_mant / 16;
  //printf ("div10 (2): s_mant: %.8llx\n", s_mant);
  s_mant += s_mant / 256;
  //printf ("div10 (3): s_mant: %.8llx\n", s_mant);
  s_mant += s_mant / 65536;
  //printf ("div10 (4): s_mant: %.8llx\n", s_mant);
  
  while ((s_mant & ~0xffffffffull) != 0)
    {
      s_mant >>= 1;
      s_exp++;
    }

  //printf ("div10: s_mant: %.8llx, s_exp: %d\n", s_mant, s_exp);

  print_unpacked ("div");
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

      return repack (a_mant << 8, a_exp, a_sign);
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

      return repack (b_mant << 8, b_exp, b_sign);
    }
}

void
fp_add_s (uint64_t b_mant, int16_t b_exp, bool b_sign)
{
  if (s_exp > b_exp)
    {
      if (s_exp > b_exp + 31)
        return;

      s_mant += b_mant >> (s_exp - b_exp);

      while ((s_mant & ~0xffffffffull) != 0)
        {
	  s_mant >>= 1;
	  s_exp++;
	}
    }
  else
    {
      if (b_exp > s_exp + 31)
        goto out;
      
      b_mant += s_mant >> (b_exp - s_exp);
      
      while ((b_mant & ~0xffffffffull) != 0)
        {
	  b_mant >>= 1;
	  b_exp++;
	}

    out:
      s_mant = b_mant;
      s_exp = b_exp;
    }
}

void
fp_print (FILE *f, sftype x)
{
  int dec_exp, place;
  bool print_exp = false;
  int i, sigfigs = 8;
  uint64_t bias_mant;
  int16_t bias_exp;
  bool bias_sign;
  static char output[16];
  int optr = 0;

  unpack (float_to_sftype (5.0));
  for (i = 0; i < sigfigs; i++)
    div10 ();

  bias_mant = s_mant;
  bias_exp = s_exp;
  bias_sign = s_sign;
    
  unpack (x);

  if (s_exp == 0 && s_mant == 0)
    {
      fprintf (f, "0");
      return;
    }

  dec_exp = 0;
  
  print_unpacked ("start");
  
  while (s_exp < 127)
    {
      mul10 ();
      dec_exp--;
    }
  
  while (s_exp > 130 || (s_exp == 130 && s_mant >= 0xa0000000))
    {
      div10 ();
      dec_exp++;
    }

  fp_add_s (bias_mant, bias_exp, bias_sign);
  
  /* Did we overflow?  */
  while (s_exp > 130 || (s_exp == 130 && s_mant >= 0xa0000000))
    {
      div10 ();
      dec_exp++;
    }
  
  if (dec_exp < -3 || dec_exp > 8)
    print_exp = true;
  
  if (dec_exp < 0 && !print_exp)
    {
      output[optr++] = '0';
      output[optr++] = '.';
      for (place = dec_exp; place < -1; place++)
        output[optr++] = '0';
    }
  
  //printf ("dec_exp: %d\n", dec_exp);
    
  for (place = 0;
       place < sigfigs || (!print_exp && place <= dec_exp);
       place++)
    {
      uint32_t digit;
      bool lastplace = (place == sigfigs - 1);
            
      denormalize ();
      
      digit = s_mant >> 28;
      
      output[optr++] = digit + '0';

      s_mant &= 0x0fffffffull;

      if ((print_exp && place == 0 && !lastplace)
	  || (!print_exp && place == dec_exp && !lastplace))
	output[optr++] = '.';
            
      mul10 ();
    }

  place--;

  while (optr > 0 && output[optr - 1] == '0' && place > dec_exp)
    optr--, place--;
  
  if (optr > 0 && output[optr - 1] == '.')
    optr--;

  output[optr] = '\0';
  
  if (print_exp)
    {
      output[optr++] = 'E';
      optr += sprintf (&output[optr], "%d", dec_exp);
    }
  
  fputs (output, f);
}


int main (void)
{
  float q;
  int i;
  
  fp_print (stdout, float_to_sftype (1.0));
  fputc ('\n', stdout);

  fp_print (stdout, float_to_sftype (1.001));
  fputc ('\n', stdout);

  fp_print (stdout, float_to_sftype (9.9999));
  fputc ('\n', stdout);

  fp_print (stdout, float_to_sftype (0.001));
  fputc ('\n', stdout);

  fp_print (stdout, float_to_sftype (10));
  fputc ('\n', stdout);
  
  fp_print (stdout, float_to_sftype (1.1));
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

  fp_print (stdout, float_to_sftype (8388608));
  fputc ('\n', stdout);

  fp_print (stdout, float_to_sftype (16777216));
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
