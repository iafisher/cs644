#include "cs644.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


void cs644_bail(const char* msg) {
  fprintf(stderr, "fatal: %s\n", msg);
  exit(1);
}

void* oom_if_null(void* p) {
  if (p == NULL) {
    cs644_bail("out of memory");
  }
  return p;
}

void cs644_bail_if_err(long long r, const char* s) {
  if (r < 0) {
    perror(s);
    exit(1);
  }
}

void* cs644_malloc_or_bail(size_t n) {
  return oom_if_null(malloc(n));
}

void* cs644_realloc_or_bail(void* p, size_t n) {
  return oom_if_null(realloc(p, n));
}

struct cs644_str cs644_str_new() {
  return (struct cs644_str){
    .data = NULL,
    .len = 0,
    .capacity = 0,
  };
}

void cs644_str_append(struct cs644_str* s, const char* data, size_t n) {
  size_t free = s->capacity - s->len;
  if (free < n) {
    size_t newcap = s->len + n;
    s->data = cs644_realloc_or_bail(s->data, newcap);
    s->capacity = newcap;
  }

  memcpy(s->data + s->len, data, n);
  s->len += n;
}

ssize_t cs644_str_find(struct cs644_str s, char c) {
  for (size_t i = 0; i < s.len; i++) {
    if (s.data[i] == c) {
      return i;
    }
  }
  return -1;
}

struct cs644_int_result cs644_str_to_int(const char* s) {
  struct cs644_int_result invalid = { .ok = false, .r = 0 };
  if (s[0] == '\0') {
    return invalid;
  }

  char* endptr;
  long long r = strtoll(s, &endptr, 10);
  if (endptr[0] != '\0') {
    return invalid;
  }

  return (struct cs644_int_result){ .ok = true, .r = r };
}

long long cs644_str_to_int_or_bail(const char* s) {
  struct cs644_int_result r = cs644_str_to_int(s);
  if (!r.ok) {
    fprintf(stderr, "fatal: could not parse string as integer: %s\n", s);
    exit(1);
  }
  return r.r;
}
