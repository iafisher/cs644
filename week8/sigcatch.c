#include <signal.h>
#include <stdio.h>
#include <unistd.h>
#include "cs644.h"

void my_handler(int signo) {
  // printf("received signal: %d\n", signo);
  const char msg[] = "received signal\n";
  write(2, msg, sizeof msg);
}

int main() {
  BAIL_IF_ERR(sigaction(SIGUSR1, &(struct sigaction){.sa_handler=my_handler}, NULL));
  pause();
  return 0;
}
