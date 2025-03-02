#include "cs644.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


void bail(const char* msg) {
  fprintf(stderr, "fatal: %s\n", msg);
  exit(1);
}

void* oom_if_null(void* p) {
  if (p == NULL) {
    bail("out of memory");
  }
  return p;
}

void handle_err(long long r, const char* s) {
  if (r < 0) {
    perror(s);
    exit(1);
  }
}

void* malloc_s(size_t n) {
  return oom_if_null(malloc(n));
}

void* realloc_s(void* p, size_t n) {
  return oom_if_null(realloc(p, n));
}

struct lpstr lpstr_new() {
  return (struct lpstr){
    .data = NULL,
    .len = 0,
    .capacity = 0,
  };
}

void lpstr_append(struct lpstr* s, const char* data, size_t n) {
  size_t free = s->capacity - s->len;
  if (free < n) {
    size_t newcap = s->len + n;
    s->data = realloc_s(s->data, newcap);
    s->capacity = newcap;
  }

  memcpy(s->data + s->len, data, n);
  s->len += n;
}

ssize_t lpstr_find(struct lpstr s, char c) {
  for (size_t i = 0; i < s.len; i++) {
    if (s.data[i] == c) {
      return i;
    }
  }
  return -1;
}
