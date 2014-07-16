#ifndef _STRING_H
#define _STRING_H 1

#include <stddef.h>

extern void *memset (void *, int, size_t);
extern void *memcpy (void *, const void *, size_t);
extern int memcmp (const void *, const void *, size_t);
extern int strcmp (const char *, const char *);
extern size_t strlen (const char *s);
extern char *strcpy (char *, const char *);
extern void *memmove (void *, const void *, size_t);
extern int strncmp (const char *, const char *, size_t);
extern char *strncpy (char *, const char *, size_t);
extern char *strchr (const char *s, int c);
extern char *strrchr (const char *s, int c);

#endif
