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
  BAIL_IF_ERR(sigaction(SIGUSR1, &(struct sigaction){.sa_handler = handler }, NULL));

  sigset_t set, oldset;
  sigemptyset(&set);
  sigaddset(&set, SIGUSR1);
  BAIL_IF_ERR(sigprocmask(SIG_BLOCK, &set, &oldset));

  printf("Send SIGUSR1 twice (pid=%d), then press <Enter> to unmask the signal.\n", getpid());
  getchar();
  sigsuspend(&oldset);
  sleep(2);
  return 0;
}
