#ifndef _STDIO_H
#define _STDIO_H 1

#include <stdarg.h>
#include <stddef.h>

typedef int FILE;

extern FILE *stdin;
extern FILE *stdout;
extern FILE *stderr;

#define SEEK_SET 0
#define SEEK_CUR 1
#define SEEK_END 2

#define EOF (-1)

extern char __m65x_char_to_file;

extern int fputc (int, FILE *);
extern int fputs (const char *, FILE *);
#define putc(C, F) fputc ((C), (F))
extern int putchar (int);
extern int puts (const char *);
extern int vfprintf (FILE *, const char *, va_list);
extern int fprintf (FILE *, const char *, ...);
extern int printf (const char *, ...);
extern int sprintf (char *, const char *, ...);
extern int vsprintf (char *, const char *, va_list);
extern int fflush (FILE *);
extern int fseek (FILE *, long, int);
extern size_t fwrite (const void *, size_t, size_t, FILE *);
extern long ftell (FILE *);
extern void rewind (FILE *);

#endif
