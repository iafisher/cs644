#include <ctype.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

void usage(char* argv[]) {
  fprintf(stderr, "usage: %s <filename> <bufsz>\n", argv[0]);
}

int is_flag(const char* arg) {
  return arg[0] == '-';
}

int main(int argc, char* argv[]) {
  // Solution to homework exercise #7
  // https://iafisher.com/cs644/spring2025/week2
  const char* filename;
  size_t bufsz = 4096;
  
  // Basic command-line validation:
  //  - first argument is filename
  //  - second argument is buffer size
  // One or both can be omitted. If any argument looks like a flag, abort.
  if (argc == 1) {
    filename = "/usr/share/cs644/bigfile.txt";
  } else if (argc == 2 && !is_flag(argv[1])) {
    filename = argv[1];
  } else if (argc == 3 && !is_flag(argv[1]) && !is_flag(argv[2])) {
    filename = argv[1];
    long long n = atoll(argv[2]);
    if (n <= 0) {
      usage(argv);
      return 1;
    }
    bufsz = (size_t)n;
  } else {
    usage(argv);
    return 1;
  }

  int fd = open(filename, O_RDONLY);
  if (fd < 0) {
    perror("open");
    return 1;
  }

  size_t nbytes = 0;
  size_t space_count = 0;

  // In classic C, we'd need to use `malloc` since the array size is not known at compile
  // time. But since C99, we can have flexible stack-allocated arrays.
  char buf[bufsz];

  while (1) {
    ssize_t bytes_read = read(fd, buf, bufsz);
    if (bytes_read == 0) {
      break;
    } else if (bytes_read < 0) {
      perror("read");
      return 1;
    } else {
      nbytes += bytes_read;
    }

    for (size_t i = 0; i < bytes_read; i++) {
      if (isspace(buf[i])) {
        space_count += 1;
      }
    }
  }

  int err = close(fd);
  if (err < 0) {
    perror("close");
    return 1;
  }

  printf("Bytes:  %lu\n", nbytes);
  printf("Spaces: %lu\n", space_count);
  return 0;
}
