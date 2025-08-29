#include <signal.h>
#include <stdio.h>
#include <unistd.h>
#include "cs644.h"

void handler(int signo) {
  char s[] = "signal handler\n";
  write(1, s, sizeof s);
}

int main() {
  int r = sigaction(SIGUSR1, &(struct sigaction){.sa_handler = handler }, NULL);
  cs644_bail_if_err(r, "sigaction");

  sigset_t set, oldset;
  sigemptyset(&set);
  sigaddset(&set, SIGUSR1);
  r = sigprocmask(SIG_BLOCK, &set, &oldset);
  cs644_bail_if_err(r, "sigprocmask");

  printf("Send me a SIGUSR1 (pid=%d).\n", getpid());

  while (1) {
    sigset_t pending;
    r = sigpending(&pending);
    cs644_bail_if_err(r, "sigpending");
    if (sigismember(&pending, SIGUSR1)) {
      break;
    } else {
      sleep(1);
    }
  }

  pid_t pid = fork();
  cs644_bail_if_err(pid, "fork");
  if (pid == 0) {
    // child
    sigset_t myset;
    r = sigprocmask(0, NULL, &myset);
    cs644_bail_if_err(r, "sigprocmask");

    if (sigismember(&myset, SIGUSR1)) {
      puts("SIGUSR1 mask WAS inherited across fork().");
    } else {
      puts("SIGUSR1 mask WAS NOT inherited across fork().");
    }

    struct sigaction act;
    r = sigaction(SIGUSR1, NULL, &act);
    cs644_bail_if_err(r, "sigaction");

    if (act.sa_handler == handler) {
      puts("SIGUSR1 handler WAS inherited across fork().");
    } else {
      puts("SIGUSR1 handler WAS NOT inherited across fork().");
    }

    sigset_t pending;
    r = sigpending(&pending);
    cs644_bail_if_err(r, "sigpending");

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
