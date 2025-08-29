#include <signal.h>
#include <stdio.h>
#include <sys/select.h>
#include <sys/signalfd.h>
#include <unistd.h>
#include "cs644.h"

#define STDIN 0

int main() {
  sigset_t set;
  int r = sigemptyset(&set);
  cs644_bail_if_err(r, "sigemptyset");
  r = sigaddset(&set, SIGUSR1);
  cs644_bail_if_err(r, "sigaddset");
  r = sigaddset(&set, SIGTERM);
  cs644_bail_if_err(r, "sigaddset");

  r = sigprocmask(SIG_BLOCK, &set, NULL);
  cs644_bail_if_err(r, "sigprocmask");

  int sigfd = signalfd(-1, &set, 0);
  cs644_bail_if_err(sigfd, "signalfd");

  printf("Send SIGUSR1 or enter input (or SIGTERM to terminate). (pid=%d)\n", getpid());

  while (1) {
    fd_set readfds;
    FD_ZERO(&readfds);
    FD_SET(sigfd, &readfds);
    FD_SET(STDIN, &readfds);
    r = select(sigfd + 1, &readfds, NULL, NULL, NULL);
    cs644_bail_if_err(r, "select");

    puts("select() returned");

    if (FD_ISSET(STDIN, &readfds)) {
      puts("received input from stdin");

      // read input so STDIN isn't perpetually ready
      char buf[1024];
      ssize_t nread = read(STDIN, buf, sizeof buf);
      cs644_bail_if_err(nread, "read");
    }

    if (FD_ISSET(sigfd, &readfds)) {
      struct signalfd_siginfo info;
      ssize_t nread = read(sigfd, &info, sizeof info);
      cs644_bail_if_err(nread, "read");

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
