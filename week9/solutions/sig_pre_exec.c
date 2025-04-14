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

  char* pathname = "./builddir/sig_post_exec";
  char* const argv[] = {pathname, NULL};
  r = execve(pathname, argv, NULL);
  cs644_bail_if_err(r, "execve");
}
