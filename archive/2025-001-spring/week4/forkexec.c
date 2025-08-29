#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

extern char **environ;

int main() {
  pid_t pid = fork();
  if (pid < 0) {
    perror("fork");
    return 1;
  } else if (pid == 0) {
    // child
    pid_t mypid = getpid();
    pid_t parentpid = getppid();
    printf("I am the child. My PID is %d and my parent's is %d.\n", mypid, parentpid);
    char* argv[] = {"/usr/bin/echo", "hello, world", NULL};
    execve("/usr/bin/echo", argv, environ);
    perror("execve");
    _exit(1);
  } else {
    // parent
    pid_t mypid = getpid();
    printf("I am the parent. My PID is %d and my child's is %d.\n", mypid, pid);
    int wstatus;
    pid_t st = waitpid(pid, &wstatus, 0);
    if (st < 0) {
      perror("waitpid");
      return 1;
    } else {
      printf("child exited: wstatus=%d\n", wstatus);
    }
  }
}
