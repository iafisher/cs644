#ifndef CS644_H
#define CS644_H

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
void cs644_str_free(struct cs644_str);

char* cs644_readfile(int fd);

struct cs644_str_vec {
  char** data;
  size_t len;
  size_t capacity;
};

struct cs644_str_vec cs644_str_vec_new();
char* cs644_str_vec_get(struct cs644_str_vec, size_t);
size_t cs644_str_vec_len(struct cs644_str_vec);
struct cs644_str_vec cs644_str_vec_split(const char*, char);
void cs644_str_vec_append(struct cs644_str_vec*, char*);
void cs644_str_vec_free(struct cs644_str_vec);

struct cs644_int_result {
  bool ok;
  long long r;
};

struct cs644_int_result cs644_str_to_int(const char*);
long long cs644_str_to_int_or_bail(const char*);

void cs644_sleep_millis(unsigned int);

bool cs644_check_n_args(int n, int argc, char* argv[]);

#endif // CS644_H
