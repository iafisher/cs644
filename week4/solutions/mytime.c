#include <sys/resource.h>
#include <sys/wait.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include "cs644.h"

double timeval_to_secs(struct timeval tval) {
  return (double)tval.tv_sec + ((double)tval.tv_usec / 1000000.0);
}

double timespec_to_secs(struct timespec tspec) {
  return (double)tspec.tv_sec + ((double)tspec.tv_nsec / 1000000000.0);
}

int main(int argc, char* argv[]) {
  if (argc < 2) {
    return 1;
  }

  struct timespec start;
  int r = clock_gettime(CLOCK_REALTIME, &start);
  cs644_bail_if_err(r, "clock_gettime");

  pid_t pid = fork();
  cs644_bail_if_err(pid, "fork");
  if (pid == 0) {
    // child
    execvp(argv[1], argv + 1);
    _exit(127);
  } else {
    // parent
    int wstatus;
    struct rusage rusage;
    int r = wait4(pid, &wstatus, 0, &rusage);
    cs644_bail_if_err(r, "wait4");
    struct timespec end;
    r = clock_gettime(CLOCK_REALTIME, &end);
    cs644_bail_if_err(r, "clock_gettime");

    double real_secs = timespec_to_secs(end) - timespec_to_secs(start);
    printf("real   %.2f secs\n", real_secs);
    printf("user   %.2f secs\n", timeval_to_secs(rusage.ru_utime));
    printf("sys    %.2f secs\n", timeval_to_secs(rusage.ru_stime));
  }
}
