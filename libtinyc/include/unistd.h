#ifndef _UNISTD_H
#define _UNISTD_H 1

typedef int pid_t;

extern char **environ;

extern pid_t fork (void);
extern int execl (const char *, const char *, ...);
extern int execlp (const char *, const char *, ...);
extern int execle (const char *, const char *, ...);
extern int execv (const char *path, char *const argv[]);
extern int execvp (const char *, char *const argv[]);
extern int execvpe (const char *, char *const argv[], char *const envp[]);
extern int execve (const char *, char *const argv[], char *const envp[]);

#endif
