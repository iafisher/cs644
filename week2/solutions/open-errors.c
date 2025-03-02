#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>

int main() {
  // Solution to homework exercise #6
  // https://iafisher.com/cs644/spring2025/week2
  int r = open("/root/hello.txt", O_WRONLY | O_CREAT, 0644);
  assert(r < 0);
  assert(errno == EACCES);
  perror("open(\"/root/hello.txt\")");

  r = open("/usr/share/cs644/code/README.md", O_WRONLY | O_CREAT | O_EXCL, 0644);
  assert(r < 0);
  assert(errno == EEXIST);
  perror("open(\"/usr/share/cs644/code/README.md\")");

  r = open("/non/existent/path", O_RDONLY);
  assert(r < 0);
  assert(errno == ENOENT);
  perror("open(\"/non/existent/path\")");
  return 0;
}
