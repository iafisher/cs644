#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include "cs644.h"

int main(int argc, char* argv[]) {
  if (argc != 2) {
    return 1;
  }

  int fd = open(argv[1], O_RDONLY | O_NONBLOCK);
  cs644_bail_if_err(fd, "open");

  char buf[1];
  for (int i = 0; i < 10; i++) {
    ssize_t nread = read(fd, buf, 1);
    if (nread < 0) {
      if (errno == EAGAIN || errno == EWOULDBLOCK) {
        puts("nothing to read, trying again in a few seconds");
        sleep(3);
      } else {
        cs644_bail_if_err(nread, "read");
      }
    } else {
      printf("read: '%c'\n", *buf);
    }
  }

  close(fd);
  return 0;
}
