#include <assert.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "cs644.h"

#define BUFSZ 100000

const char* TEMPFILE = "/tmp/ian-test";

void fill_with_char(char*, size_t, char);
void check_all_char(const char*, size_t, char);
void clean_up(void);

int main(int argc, char* argv[]) {
  // Solution to homework exercise #10
  // https://iafisher.com/cs644/spring2025/week2

  int fd = open(TEMPFILE, O_CREAT | O_TRUNC | O_RDWR, 0600);
  cs644_bail_if_err(fd, "open");
  // Call `clean_up` (which removes the temporary file) when the program exits,
  // (`atexit` is a C standard library feature.)
  atexit(clean_up);

  // First, fill the file with spaces and flush to disk.
  char s[BUFSZ];
  fill_with_char(s, BUFSZ, ' ');
  long long r = write(fd, s, BUFSZ);
  assert(r == BUFSZ);
  r = fsync(fd);
  cs644_bail_if_err(r, "fsync");

  // Next, rewind and fill the file with X's, but don't flush.
  r = lseek(fd, 0, SEEK_SET);
  cs644_bail_if_err(r, "lseek");
  fill_with_char(s, BUFSZ, 'X');
  r = write(fd, s, BUFSZ);
  assert(r == BUFSZ);

  // Rewind again and start reading.
  r = lseek(fd, 0, SEEK_SET);
  cs644_bail_if_err(r, "lseek");

  char s2[BUFSZ];
  while (1) {
    ssize_t nread = read(fd, s2, BUFSZ);
    cs644_bail_if_err(nread, "read");
    if (nread == 0) {
      break;
    }

    check_all_char(s2, nread, 'X');
  }

  puts("read-after-write: success");

  r = close(fd);
  cs644_bail_if_err(r, "close");

  return 0;
}

void fill_with_char(char* s, size_t n, char ch) {
  for (size_t i = 0; i < n; i++) {
    s[i] = ch;
  }
}

void check_all_char(const char* s, size_t n, char ch) {
  for (size_t i = 0; i < n; i++) {
    if (s[i] != ch) {
      puts("read-after-write: FAILURE");
      exit(1);
    }
  }
}

void clean_up() {
  // Don't check error code here as clean-up is best-effort.
  unlink(TEMPFILE);
}
