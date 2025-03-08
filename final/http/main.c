#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/file.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>

#include "cs644.h"


#define LOG_FILE_DIR       "/home/ian/"
#define LOG_FILE_BASENAME  "http"
#define LOG_FILE_SUFFIX    ".log"
#define LOG_FILE_FMT              (LOG_FILE_DIR LOG_FILE_BASENAME ".%d" LOG_FILE_SUFFIX)
#define LOG_FILE_FMT_LEN  (sizeof  LOG_FILE_DIR LOG_FILE_BASENAME ".X"  LOG_FILE_SUFFIX)

const char* LOG_FILE = LOG_FILE_DIR LOG_FILE_BASENAME LOG_FILE_SUFFIX;

void usage(void);

#define MAX_LINE 1024

void append_to_log() {
  int fd = open(LOG_FILE, O_CREAT | O_APPEND | O_WRONLY, 0644);
  handle_err(fd, "open");

  while (1) {
    struct timeval tv;
    long long r = gettimeofday(&tv, NULL);
    handle_err(r, "gettimeofday");

    char line[MAX_LINE];
    int bytes_needed = snprintf(line, MAX_LINE, "%ld server heartbeat\n", tv.tv_sec);
    if (bytes_needed >= MAX_LINE) {
      bail("buffer too small (snprintf)");
    }

    r = flock(fd, LOCK_EX);
    handle_err(r, "flock(LOCK_EX)");
    r = write(fd, line, bytes_needed);
    handle_err(r, "write");
    r = flock(fd, LOCK_UN);
    handle_err(r, "flock(LOCK_UN)");

    struct timespec duration = { .tv_sec = 1, .tv_nsec = 0};
    r = nanosleep(&duration, NULL);
    handle_err(r, "nanosleep");
  }

  int r = close(fd);
  handle_err(r, "close");
}

#define READ_BUFSZ 4096

unsigned long count_log_lines() {
  int fd = open(LOG_FILE, O_RDONLY);
  handle_err(fd, "open");

  char buf[READ_BUFSZ];

  unsigned long line_count = 0;
  while (1) {
    int r = flock(fd, LOCK_SH);
    handle_err(r, "flock(LOCK_SH)");
    ssize_t nread = read(fd, buf, READ_BUFSZ);
    handle_err(nread, "read");
    r = flock(fd, LOCK_UN);
    handle_err(r, "flock(LOCK_UN)");

    if (nread == 0) {
      // TODO: this assumes that the file terminates with a newline (which our logging
      // function ensures, but would be good to handle this case).
      break;
    }

    for (size_t i = 0; i < nread; i++) {
      if (buf[i] == '\n') {
        line_count++;
      }
    }
  }
  return line_count;
}

void log_file_n(int n, char* buf) {
  assert(n >= 0);
  assert(n < 10);

  if (n == 0) {
    strcpy(buf, LOG_FILE);
    return;
  }

  size_t bytes = LOG_FILE_FMT_LEN;
  int r = snprintf(buf, bytes, LOG_FILE_FMT, n);
  assert(r < bytes);
}

void rotate_logs() {
  char buf1[LOG_FILE_FMT_LEN];
  char buf2[LOG_FILE_FMT_LEN];
  log_file_n(5, buf1);
  int r = unlink(buf1);
  if (r < 0 && errno != ENOENT) {
    handle_err(r, "unlink");
  }

  for (int i = 4; i >= 0; i--) {
    log_file_n(i, buf1);
    log_file_n(i + 1, buf2);
    int r = rename(buf1, buf2);
    if (r < 0 && errno != ENOENT) {
      handle_err(r, "rename");
    }
  }
}

int main(int argc, char* argv[]) {
  if (argc < 2) {
    usage();
  }

  const char* cmd = argv[1];
  if (strcmp(cmd, "run") == 0) {
    if (argc != 2) {
      usage();
    }

    append_to_log();
  } else if (strcmp(cmd, "count") == 0) {
    if (argc != 2) {
      usage();
    }

    unsigned long n = count_log_lines();
    printf("log lines: %lu\n", n);
  } else if (strcmp(cmd, "rotate") == 0) {
    if (argc != 2) {
      usage();
    }

    rotate_logs();
  } else {
    usage();
  }

  return 0;
}

void usage() {
  fputs("usage: ./http [cmd]\n", stderr);
  exit(1);
}
