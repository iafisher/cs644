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
  puts("==> sig_pre_exec");
  BAIL_IF_ERR(sigaction(SIGUSR1, &(struct sigaction){.sa_handler = handler }, NULL));
  BAIL_IF_ERR(sigaction(SIGUSR2, &(struct sigaction){.sa_handler = SIG_IGN }, NULL));

  sigset_t set, oldset;
  sigemptyset(&set);
  sigaddset(&set, SIGUSR1);
  BAIL_IF_ERR(sigprocmask(SIG_BLOCK, &set, &oldset));
  puts("SIGUSR1 mask set.");

  BAIL_IF_ERR(raise(SIGUSR1));
  puts("Sent myself a SIGUSR1.");

  char* pathname = "./builddir/sig_post_exec";
  char* const argv[] = {pathname, NULL};
  puts("Calling execve!");
  BAIL_IF_ERR(execve(pathname, argv, NULL));
}
