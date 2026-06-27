/**
 * https://iafisher.com/cs644/fall2025/week8
 */
#include <signal.h>
#include <stdio.h>
#include <unistd.h>
#include "cs644.h"

int main() {
  puts("==> sig_post_exec");
  sigset_t myset;
  BAIL_IF_ERR(sigprocmask(0, NULL, &myset));

  if (sigismember(&myset, SIGUSR1)) {
    puts("SIGUSR1 mask WAS inherited across exec().");
  } else {
    puts("SIGUSR1 mask WAS NOT inherited across exec().");
  }

  sigset_t pending;
  BAIL_IF_ERR(sigpending(&pending));

  if (sigismember(&pending, SIGUSR1)) {
    puts("SIGUSR1 pending signal WAS inherited across exec().");
  } else {
    puts("SIGUSR1 pending signal WAS NOT inherited across exec().");
  }

  struct sigaction act;
  BAIL_IF_ERR(sigaction(SIGUSR2, NULL, &act));

  if (act.sa_handler == SIG_IGN) {
    puts("SIGUSR2 disposition WAS inherited across exec().");
  } else {
    puts("SIGUSR2 disposition WAS NOT inherited across exec().");
  }

  return 0;
}
