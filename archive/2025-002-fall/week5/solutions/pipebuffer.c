/**
 * https://iafisher.com/cs644/fall2025/week5
 */
#include <stdio.h>
#include <unistd.h>
#include "cs644.h"

#define BUFSZ 8

int main() {
  int pipefd[2];
  BAIL_IF_ERR(pipe(pipefd));

  puts("Possible outcomes:");
  puts("  - Nothing is printed -- no pipe buffering");
  puts("  - A fixed number of lines are printed -- fixed-size buffer");
  puts("  - Lines are printed forever -- an infinite buffer");
  puts("");

  char buf[BUFSZ] = {0};
  ssize_t nwritten = 0;
  while (1) {
    nwritten += BAIL_IF_ERR(write(pipefd[1], buf, BUFSZ));
    printf("%ld\n", nwritten);
  }
}
