#include <fcntl.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <unistd.h>

#include "cs644.h"


const char* LOG_FILE = "/home/ian/http.log";

void usage(void);

#define MAX_LINE 1024

void append_to_log() {
  int fd = open(LOG_FILE, O_CREAT | O_APPEND | O_WRONLY, 0644);
  handle_err(fd, "open");

  struct timeval tv;
  long long r = gettimeofday(&tv, NULL);
  handle_err(r, "gettimeofday");

  char line[MAX_LINE];
  int bytes_needed = snprintf(line, MAX_LINE, "%ld server started\n", tv.tv_sec);
  if (bytes_needed >= MAX_LINE) {
    bail("buffer too small (snprintf)");
  }

  r = write(fd, line, bytes_needed);
  handle_err(r, "write");

  r = close(fd);
  handle_err(fd, "close");
}

#define READ_BUFSZ 4096

unsigned long count_log_lines() {
  int fd = open(LOG_FILE, O_RDONLY);
  handle_err(fd, "open");

  char buf[READ_BUFSZ];

  unsigned long line_count = 0;
  while (1) {
    ssize_t nread = read(fd, buf, READ_BUFSZ);
    handle_err(nread, "read");
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
  } else {
    usage();
  }

  return 0;
}

void usage() {
  fputs("usage: ./http [cmd]\n", stderr);
  exit(1);
}
