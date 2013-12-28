#ifndef _STDLIB_H
#define _STDLIB_H 1

extern void *malloc (size_t);
extern void free (void *);
extern void *calloc (size_t, size_t);
extern void *realloc (void *, size_t);
extern void exit (int);
extern void abort (void);

#endif
