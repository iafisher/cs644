#ifndef CS644_LPSTR_H
#define CS644_LPSTR_H

#include <stdbool.h>
#include <stddef.h>
#include <sys/types.h>

void bail(const char*);
void* oom_if_null(void* p);
void handle_err(long long r, const char* s);
void* malloc_s(size_t n);
void* realloc_s(void* p, size_t n);

struct lpstr {
  char* data;
  size_t len;
  size_t capacity;
};

struct lpstr lpstr_new();
void lpstr_append(struct lpstr* s, const char* data, size_t n);
ssize_t lpstr_find(struct lpstr s, char c);

struct cs644_int_result {
  bool ok;
  long long r;
};

struct cs644_int_result cs644_str_to_int(const char*);
long long cs644_str_to_int_or_bail(const char*);

#endif // CS644_LPSTR_H
