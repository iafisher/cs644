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

  printf("Send SIGUSR1 twice (pid=%d), then press <Enter> to unmask the signal.\n", getpid());
  getchar();
  sigsuspend(&oldset);
  sleep(2);
  return 0;
}
