/**
 * https://iafisher.com/cs644/fall2025/week3
 */

#define _GNU_SOURCE // necessary to expose `getdents64`
#include <dirent.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/sendfile.h>
#include <sys/stat.h>
#include <sys/types.h> // for `ino64_t` and `off64_t`
#include <unistd.h>
#include "cs644.h"

// as specified in getdents64(2)
struct linux_dirent64 {
  ino64_t d_ino;
  off64_t d_off;
  unsigned short d_reclen;
  unsigned char d_type;
  char d_name[];
};

void cp_file(int fd, int dirfd, const char* name) {
  // NOTE: should probably copy the mode bits from the source file...
  int outfd = openat(dirfd, name, O_WRONLY | O_CREAT | O_EXCL, 0644);
  cs644_bail_if_err(outfd, "openat");

  while (1) {
    ssize_t r = sendfile(outfd, fd, NULL, 8192);
    cs644_bail_if_err(r, "sendfile");
    if (r == 0) {
      break;
    }
  }

  int r = close(outfd);
  cs644_bail_if_err(r, "close");
}

// `cp_dir(foo, bar)` will create a copy of `foo/` at `bar/foo/`.
void cp_dir(int srcfd, int destfd) {
  size_t buflen = 4096;
  char buf[buflen];

  // loop over calls to `getdents64`
  while (1) {
    ssize_t bytes_read = getdents64(srcfd, buf, buflen);
    cs644_bail_if_err(bytes_read, "getdents64");
    if (bytes_read == 0) {
      break;
    }

    // loop over entries returned by `getdents64`
    size_t offset = 0;
    while (offset < bytes_read) {
      struct linux_dirent64* ent = (struct linux_dirent64*)(buf + offset);
      offset += ent->d_reclen;

      if (strcmp(ent->d_name, ".") == 0 || strcmp(ent->d_name, "..") == 0) {
        continue;
      }

      switch (ent->d_type) {
        case DT_DIR:
          int r = mkdirat(destfd, ent->d_name, 0755);
          cs644_bail_if_err(r, "mkdirat");

          int srcfd2 = openat(srcfd, ent->d_name, O_RDONLY | O_DIRECTORY);
          cs644_bail_if_err(srcfd2, "openat");

          int destfd2 = openat(destfd, ent->d_name, O_RDONLY | O_DIRECTORY);
          cs644_bail_if_err(destfd2, "openat");

          printf("recursively copying directory: %s\n", ent->d_name);
          cp_dir(srcfd2, destfd2);
          break;
        case DT_REG:
          printf("copying file: %s\n", ent->d_name);
          int fd = openat(srcfd, ent->d_name, O_RDONLY);
          cs644_bail_if_err(fd, "openat");
          cp_file(fd, destfd, ent->d_name);
          r = close(fd);
          cs644_bail_if_err(r, "close");
          break;
        case DT_LNK:
          // TODO
          break;
        default:
          cs644_bail("not copying unknown file type");
      }
    }
  }
}


int main(int argc, char* argv[]) {
  if (!cs644_check_n_args(2, argc, argv)) {
    fprintf(stderr, "usage: %s SRC DEST\n", argv[0]);
    return 1;
  }

  printf("recursively copying contents of %s to %s\n", argv[1], argv[2]);
  int srcfd = open(argv[1], O_RDONLY | O_DIRECTORY);
  cs644_bail_if_err(srcfd, "open");
  int destfd = open(argv[2], O_RDONLY | O_DIRECTORY);
  cs644_bail_if_err(destfd, "open");

  cp_dir(srcfd, destfd);
  return 0;
}
