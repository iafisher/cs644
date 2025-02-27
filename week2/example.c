#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>

#define BUFSZ 4096

int main() {
  const char* filename = "/usr/share/cs644/bigfile.txt";
  int fd = open(filename, O_RDONLY);
  if (fd < 0) {
    perror("open");
    return 1;
  }

  size_t nbytes = 0;

  char buf[BUFSZ];

  while (1) {
    ssize_t bytes_read = read(fd, buf, BUFSZ);
    if (bytes_read == 0) {
      break;
    } else if (bytes_read < 0) {
      perror("read");
      return 1;
    } else {
      nbytes += bytes_read;
    }
  }

  int err = close(fd);
  if (err < 0) {
    perror("close");
    return 1;
  }

  printf("%lu\n", nbytes);
  return 0;
}
