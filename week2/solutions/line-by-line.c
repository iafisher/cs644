#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "cs644.h"

int is_flag(const char* arg) {
  return arg[0] == '-';
}

char* readline(int fd);

int main(int argc, char* argv[]) {
  const char* filename;
  if (argc == 2 && !is_flag(argv[1])) {
    filename = argv[1];
  } else {
    fprintf(stderr, "usage: %s <filename>\n", argv[0]);
    return 1;
  }

  int fd = open(filename, O_RDONLY);
  if (fd < 0) {
    perror("open");
    return 1;
  }

  char* line;
  unsigned int i = 1;
  while ((line = readline(fd)) != NULL) {
    size_t n = strlen(line);
    printf("line %u: %lu byte(s)\n", i, n);
    printf("line: %s\n", line);
    free(line);
    i++;
  }

  int err = close(fd);
  if (err < 0) {
    perror("close");
    return 1;
  }

  return 0;
}

#define NOT_DONE ((off_t)1)
#define READSZ 16

off_t build_line(struct cs644_str* s, char* buf, size_t bufsz) {
  if (bufsz == 0) {
    return 0;
  }

  cs644_str_append(s, buf, bufsz);
  ssize_t newline_pos = cs644_str_find(*s, '\n');
  if (newline_pos == -1) {
    return NOT_DONE;
  }

  // Make it null-terminated so it's a valid C string. (Remember that the string that
  // `read` returns is *not* null-terminated.) We overwrite the newline, which is
  // convenient because we don't want to return a string with a newline at the end anyway.
  s->data[newline_pos] = '\0';

  // Suppose we read "a\nb".
  //
  //   s->len = 3
  //   newline_pos = 1
  //
  // The file's cursor is at the character after 'b', and we want to set it back by one,
  // so this function should return -1 = (newline_pos - s->len) + 1.
  return (newline_pos - s->len) + 1;
}

char* readline(int fd) {
  struct cs644_str s = cs644_str_new();
  while (1) {
    char buf[READSZ];
    ssize_t nread = read(fd, buf, READSZ);
    cs644_bail_if_err(nread, "read");

    off_t lseek_offset = build_line(&s, buf, nread);
    if (lseek_offset == NOT_DONE) {
      continue;
    } else {
      off_t r = lseek(fd, lseek_offset, SEEK_CUR);
      cs644_bail_if_err(r, "lseek");
      break;
    }
  }
  return s.data;
}
