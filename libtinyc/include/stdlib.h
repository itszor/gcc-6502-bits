#ifndef _STDLIB_H
#define _STDLIB_H 1

#include <stddef.h>

extern void *malloc (size_t);
extern void free (void *);
extern void *calloc (size_t, size_t);
extern void *realloc (void *, size_t);
extern void exit (int);
extern void abort (void);
extern void srand (unsigned int);
extern int rand (void);

#define RAND_MAX 0x7fff

#endif
