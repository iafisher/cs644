#include "cs644.h"

#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>


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

void cs644_str_free(struct cs644_str s) {
  if (s.data == NULL) {
    return;
  }

  free(s.data);
}

struct cs644_str_vec cs644_str_vec_new() {
  return (struct cs644_str_vec){
    .data = NULL,
    .len = 0,
    .capacity = 0,
  };
}

char* cs644_str_vec_get(struct cs644_str_vec v, size_t i) {
  if (i >= v.len) {
    return NULL;
  }
  return v.data[i];
}

size_t cs644_str_vec_len(struct cs644_str_vec v) {
  return v.len;
}

char* copy_substring(const char* s, size_t start, size_t end_exclusive) {
  assert(start < end_exclusive);
  size_t n = end_exclusive - start;
  char* r = cs644_malloc_or_bail(n + 1);
  memcpy(r, s + start, n);
  r[n] = '\0';
  return r;
}

struct cs644_str_vec cs644_str_vec_split(const char* s, char c) {
  struct cs644_str_vec v = cs644_str_vec_new();

  size_t n = strlen(s);
  size_t start = 0;
  for (size_t i = 0; i < n; i++) {
    if (s[i] == c) {
      cs644_str_vec_append(&v, copy_substring(s, start, i));
      start = i + 1;
    }
  }

  if (start < n - 1) {
    cs644_str_vec_append(&v, copy_substring(s, start, n));
  }

  return v;
}

void cs644_str_vec_append(struct cs644_str_vec* vec, char* s) {
  if (vec->len == vec->capacity) {
    size_t newcap = vec->capacity == 0 ? 8 : (vec->capacity * 2);
    vec->data = cs644_realloc_or_bail(vec->data, newcap * sizeof *vec->data);
    vec->capacity = newcap;
  }

  vec->data[vec->len] = s;
  vec->len += 1;
}

void cs644_str_vec_free(struct cs644_str_vec vec) {
  if (vec.data == NULL) {
    return;
  }

  free(vec.data);
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

void cs644_sleep_millis(unsigned int millis) {
  struct timespec spec = { .tv_sec = millis / 1000, .tv_nsec = (millis % 1000) * 1000000};
  struct timespec rem;
  while (1) {
    int r = nanosleep(&spec, &rem);
    if (r == -1 && errno == EINTR) {
      spec = rem;
    } else {
      break;
    }
  }
}
