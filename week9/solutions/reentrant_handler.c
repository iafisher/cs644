#include <signal.h>
#include <stdio.h>
#include <unistd.h>
#include "cs644.h"

void sigusr1_handler(int signo) {
  char s[] = "enter signal handler (SIGUSR1)\n";
  write(1, s, sizeof s);

  sleep(5);

  char s2[] = "exit signal handler (SIGUSR1)\n";
  write(1, s2, sizeof s2);
}

void sigusr2_handler(int signo) {
  char s[] = "enter signal handler (SIGUSR2)\n";
  write(1, s, sizeof s);

  sleep(5);

  char s2[] = "exit signal handler (SIGUSR2)\n";
  write(1, s2, sizeof s2);
}

int main() {
  int r = sigaction(SIGUSR1, &(struct sigaction){.sa_handler = sigusr1_handler }, NULL);
  cs644_bail_if_err(r, "sigaction");
  r = sigaction(SIGUSR2, &(struct sigaction){.sa_handler = sigusr2_handler }, NULL);
  cs644_bail_if_err(r, "sigaction");

  sigset_t set, oldset;
  sigemptyset(&set);
  sigaddset(&set, SIGUSR1);
  r = sigprocmask(SIG_BLOCK, &set, &oldset);
  cs644_bail_if_err(r, "sigprocmask");

  printf("Send SIGUSR1 (pid=%d).\n", getpid());
  sigsuspend(&oldset);
  sleep(1);
  return 0;
}
