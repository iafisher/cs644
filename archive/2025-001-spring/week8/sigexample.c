#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "cs644.h"


void graceful_cleanup(int signo) {
  // PROBLEM: Shouldn't be doing non-trivial work in the signal handler.
  // Just set a flag and handle it from `main`.

  // PROBLEM: `puts` is not safe to call in signal handler.
  puts("cleaning up");
  exit(1);
}

int main() {
  struct sigaction act = { .sa_handler = graceful_cleanup };
  int r = sigaction(SIGTERM, &act, NULL);
  if (r != 0) { cs644_bail("sigaction failed"); }

  puts("ready to receive signals");
  // PROBLEM: What if SIGTERM is received right now?
  pause();

  puts("this is after pause()");
  return 0;
}
