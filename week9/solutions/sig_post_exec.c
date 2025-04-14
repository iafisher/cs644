#include <signal.h>
#include <stdio.h>
#include <unistd.h>
#include "cs644.h"

int main() {
  sigset_t myset;
  int r = sigprocmask(0, NULL, &myset);
  cs644_bail_if_err(r, "sigprocmask");

  if (sigismember(&myset, SIGUSR1)) {
    puts("SIGUSR1 mask WAS inherited across exec().");
  } else {
    puts("SIGUSR1 mask WAS NOT inherited across exec().");
  }

  sigset_t pending;
  r = sigpending(&pending);
  cs644_bail_if_err(r, "sigpending");

  if (sigismember(&pending, SIGUSR1)) {
    puts("SIGUSR1 pending signal WAS inherited across exec().");
  } else {
    puts("SIGUSR1 pending signal WAS NOT inherited across exec().");
  }

  struct sigaction act;
  r = sigaction(SIGUSR2, NULL, &act);
  cs644_bail_if_err(r, "sigaction");

  if (act.sa_handler == SIG_IGN) {
    puts("SIGUSR2 disposition WAS inherited across exec().");
  } else {
    puts("SIGUSR2 disposition WAS NOT inherited across exec().");
  }

  return 0;
}
