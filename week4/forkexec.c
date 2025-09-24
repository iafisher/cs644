#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>

extern char** environ;

int main() {
  pid_t pid = fork();
  if (pid < 0) {
    return 1;
  } else if (pid == 0) {
    // child
    printf("I am the child at %d, executing the original program.\n", getpid());
    sleep(2);
    char* pathname = "/usr/bin/echo";
    char* argv[] = {pathname, "hello", "world", NULL};
    execve(pathname, argv, environ);
    _exit(1);
  } else {
    printf("I am the parent, having just spawned child process %d.\n", pid);
    // parent
    int wstatus;
    pid_t childpid = waitpid(pid, &wstatus, 0);
    if (childpid < 0) {
      return 1;
    }
    puts("child returned into parent");
  }
}
