#include <signal.h>
#include <stdio.h>
#include <unistd.h>
#include "cs644.h"

void sigusr1_handler(int signo) {
  char s[] = "signal handler (SIGUSR1)\n";
  write(1, s, sizeof s);
}

void sigusr2_handler(int signo) {
  char s[] = "signal handler (SIGUSR2)\n";
  write(1, s, sizeof s);
}

int main() {
  int r = sigaction(SIGUSR1, &(struct sigaction){.sa_handler = sigusr1_handler }, NULL);
  cs644_bail_if_err(r, "sigaction");
  r = sigaction(SIGUSR2, &(struct sigaction){.sa_handler = sigusr2_handler }, NULL);
  cs644_bail_if_err(r, "sigaction");

  sigset_t set, oldset;
  sigemptyset(&set);
  sigaddset(&set, SIGUSR1);
  sigaddset(&set, SIGUSR2);
  r = sigprocmask(SIG_BLOCK, &set, &oldset);
  cs644_bail_if_err(r, "sigprocmask");

  printf("Send SIGUSR1 and SIGUSR2 (pid=%d), then press <Enter> to unmask the signal.\n", getpid());
  getchar();
  sigsuspend(&oldset);
  sleep(2);
  return 0;
}
