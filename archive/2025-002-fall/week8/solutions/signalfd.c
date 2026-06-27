/**
 * https://iafisher.com/cs644/fall2025/week8
 */
#include <signal.h>
#include <stdio.h>
#include <sys/select.h>
#include <sys/signalfd.h>
#include <unistd.h>
#include "cs644.h"

#define STDIN 0

int main() {
  sigset_t set;
  BAIL_IF_ERR(sigemptyset(&set));
  BAIL_IF_ERR(sigaddset(&set, SIGUSR1));
  BAIL_IF_ERR(sigaddset(&set, SIGTERM));

  BAIL_IF_ERR(sigprocmask(SIG_BLOCK, &set, NULL));

  int sigfd = BAIL_IF_ERR(signalfd(-1, &set, 0));

  printf("Send SIGUSR1 or enter input (or SIGTERM to terminate). (pid=%d)\n", getpid());

  while (1) {
    fd_set readfds;
    FD_ZERO(&readfds);
    FD_SET(sigfd, &readfds);
    FD_SET(STDIN, &readfds);
    BAIL_IF_ERR(select(sigfd + 1, &readfds, NULL, NULL, NULL));

    puts("select() returned");

    if (FD_ISSET(STDIN, &readfds)) {
      puts("received input from stdin");

      // read input so STDIN isn't perpetually ready
      char buf[1024];
      BAIL_IF_ERR(read(STDIN, buf, sizeof buf));
    }

    if (FD_ISSET(sigfd, &readfds)) {
      struct signalfd_siginfo info;
      BAIL_IF_ERR(read(sigfd, &info, sizeof info));

      switch (info.ssi_signo) {
        case SIGUSR1:
          puts("received SIGUSR1 signal");
          break;
        case SIGTERM:
          puts("received SIGTERM signal - exiting");
          return 0;
        default:
          puts("received some other signal (that's surprising...");
          break;
      }
    }
  }

  return 0;
}
