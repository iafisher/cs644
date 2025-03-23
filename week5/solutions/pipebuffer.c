#include <stdio.h>
#include <unistd.h>
#include "cs644.h"

#define BUFSZ 8

int main() {
  int pipefd[2];
  int r = pipe(pipefd);
  cs644_bail_if_err(r, "pipe");

  puts("Possible outcomes:");
  puts("  - Nothing is printed -- no pipe buffering");
  puts("  - A fixed number of lines are printed -- fixed-size buffer");
  puts("  - Lines are printed forever -- an infinite buffer");
  puts("");

  char buf[BUFSZ] = {0};
  ssize_t nwritten = 0;
  while (1) {
    ssize_t r = write(pipefd[1], buf, BUFSZ);
    cs644_bail_if_err(r, "write");

    nwritten += r;
    printf("%ld\n", nwritten);
  }
}
