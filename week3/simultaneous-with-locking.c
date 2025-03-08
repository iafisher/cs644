/**
 * Modification of homework exercise #12, using locking to synchronize.
 * https://iafisher.com/cs644/spring2025/week2
 */

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/file.h>
#include <string.h>
#include <unistd.h>

#include "cs644.h"


const char* FILENAME = "/tmp/ian-test-simultaneous";

void fill_with_char(char*, size_t, char);
int check_all_same_char(const char*, size_t);

void main_read(size_t bufsz) {
  int fd = open(FILENAME, O_RDONLY);
  handle_err(fd, "open");

  char buf[bufsz];
  size_t n_reads = 0;
  while (1) {
    long long r = flock(fd, LOCK_SH);
    handle_err(r, "flock(LOCK_SH)");
    ssize_t bytes_read = read(fd, buf, bufsz);
    r = flock(fd, LOCK_UN);
    handle_err(r, "flock(LOCK_UN)");

    n_reads++;
    handle_err(bytes_read, "read");
    if (!check_all_same_char(buf, bytes_read)) {
      printf("detected partial write after %lu read(s):\n\n  %.*s\n\n", n_reads, (int)bytes_read, buf);
      break;
    }

    r = lseek(fd, 0, SEEK_SET);
    handle_err(r, "lseek");
  }
}

void main_write(size_t bufsz) {
  pid_t pid = getpid();
  printf("write pid: %d\n", pid);

  int fd = open(FILENAME, O_WRONLY | O_TRUNC | O_CREAT, 0600);
  handle_err(fd, "open");

  char buf[bufsz];
  unsigned short offset = 0;
  while (1) {
    fill_with_char(buf, bufsz, 'a' + offset);

    long long r = flock(fd, LOCK_EX);
    handle_err(r, "flock(LOCK_EX)");
    r = write(fd, buf, bufsz);
    handle_err(r, "write");
    r = flock(fd, LOCK_UN);
    handle_err(r, "flock(LOCK_UN)");

    r = lseek(fd, 0, SEEK_SET);
    handle_err(r, "lseek");

    offset = (offset + 1) % 26;
  }
}

void usage(char* argv[]) {
  fprintf(stderr, "usage: %s {read|write} <bufsz>\n", argv[0]);
  exit(1);
}

int main(int argc, char* argv[]) {
  if (argc != 3) {
    usage(argv);
  }

  long long bufsz = atoll(argv[2]);
  if (bufsz <= 0) {
    usage(argv);
  }

  if (strcmp(argv[1], "read") == 0) {
    main_read((size_t)bufsz);
  } else if (strcmp(argv[1], "write") == 0) {
    main_write((size_t)bufsz);
  } else {
    usage(argv);
  }

  return 0;
}

void fill_with_char(char* s, size_t n, char ch) {
  for (size_t i = 0; i < n; i++) {
    s[i] = ch;
  }
}

int check_all_same_char(const char* s, size_t n) {
  if (n < 2) {
    return 1;
  }

  char ch = s[0];
  for (size_t i = 1; i < n; i++) {
    if (s[i] != ch) {
      return 0;
    }
  }

  return 1;
}
