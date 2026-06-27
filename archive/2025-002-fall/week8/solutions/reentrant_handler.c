/**
 * https://iafisher.com/cs644/fall2025/week8
 */
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
  BAIL_IF_ERR(sigaction(SIGUSR1, &(struct sigaction){.sa_handler = sigusr1_handler }, NULL));
  BAIL_IF_ERR(sigaction(SIGUSR2, &(struct sigaction){.sa_handler = sigusr2_handler }, NULL));

  sigset_t set, oldset;
  sigemptyset(&set);
  sigaddset(&set, SIGUSR1);
  BAIL_IF_ERR(sigprocmask(SIG_BLOCK, &set, &oldset));

  printf("Send SIGUSR1 (pid=%d), then try sending SIGUSR1 or SIGUSR2.\n", getpid());
  sigsuspend(&oldset);
  sleep(1);
  return 0;
}
