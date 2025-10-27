/**
 * https://iafisher.com/cs644/fall2025/week8
 */
#include <signal.h>
#include <stdio.h>
#include <unistd.h>
#include "cs644.h"

void handler(int signo) {
  char s[] = "signal handler\n";
  write(1, s, sizeof s);
}

int main() {
  puts("==> parent process");
  BAIL_IF_ERR(sigaction(SIGUSR1, &(struct sigaction){.sa_handler = handler }, NULL));

  sigset_t set, oldset;
  sigemptyset(&set);
  sigaddset(&set, SIGUSR1);
  BAIL_IF_ERR(sigprocmask(SIG_BLOCK, &set, &oldset));
  puts("SIGUSR1 mask set.");

  BAIL_IF_ERR(raise(SIGUSR1));
  puts("Sent myself a SIGUSR1.");

  sigset_t pending;
  BAIL_IF_ERR(sigpending(&pending));
  if (!sigismember(&pending, SIGUSR1)) {
    cs644_bail("sent myself a SIGUSR1 but it's not pending...");
  }

  puts("Forking!");
  pid_t pid = BAIL_IF_ERR(fork());
  if (pid == 0) {
    // child
    puts("==> child process");
    sigset_t myset;
    BAIL_IF_ERR(sigprocmask(0, NULL, &myset));

    if (sigismember(&myset, SIGUSR1)) {
      puts("SIGUSR1 mask WAS inherited across fork().");
    } else {
      puts("SIGUSR1 mask WAS NOT inherited across fork().");
    }

    struct sigaction act;
    BAIL_IF_ERR(sigaction(SIGUSR1, NULL, &act));

    if (act.sa_handler == handler) {
      puts("SIGUSR1 handler WAS inherited across fork().");
    } else {
      puts("SIGUSR1 handler WAS NOT inherited across fork().");
    }

    sigset_t pending;
    BAIL_IF_ERR(sigpending(&pending));

    if (sigismember(&pending, SIGUSR1)) {
      puts("SIGUSR1 pending signal WAS inherited across fork().");
    } else {
      puts("SIGUSR1 pending signal WAS NOT inherited across fork().");
    }

    return 0;
  } else {
    // parent
    return 0;
  }
}
