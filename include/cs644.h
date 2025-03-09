#ifndef CS644_LPSTR_H
#define CS644_LPSTR_H

#include <stdbool.h>
#include <stddef.h>
#include <sys/types.h>

void cs644_bail(const char*);
void cs644_bail_if_err(long long r, const char* s);
void* cs644_malloc_or_bail(size_t n);
void* cs644_realloc_or_bail(void* p, size_t n);

struct cs644_str {
  char* data;
  size_t len;
  size_t capacity;
};

struct cs644_str cs644_str_new();
void cs644_str_append(struct cs644_str* s, const char* data, size_t n);
ssize_t cs644_str_find(struct cs644_str s, char c);

struct cs644_int_result {
  bool ok;
  long long r;
};

struct cs644_int_result cs644_str_to_int(const char*);
long long cs644_str_to_int_or_bail(const char*);

#endif // CS644_LPSTR_H
