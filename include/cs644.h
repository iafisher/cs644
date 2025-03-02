#ifndef CS644_LPSTR_H
#define CS644_LPSTR_H

#include <stddef.h>
#include <sys/types.h>

void* oom_if_null(void* p);
void handle_err(long long r, const char* s);
void* realloc_s(void* p, size_t n);

struct lpstr {
  char* data;
  size_t len;
  size_t capacity;
};

struct lpstr lpstr_new();
void lpstr_append(struct lpstr* s, const char* data, size_t n);
ssize_t lpstr_find(struct lpstr s, char c);

#endif // CS644_LPSTR_H
