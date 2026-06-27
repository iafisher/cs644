/**
 * https://iafisher.com/cs644/fall2025/week3
 */

#include <dirent.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/sendfile.h>
#include <sys/stat.h>
#include <unistd.h>
#include "cs644.h"
#include "cs644_topfiles.h"


void searchdir(int dirfd, struct topfiles_list* topfiles) {
  size_t buflen = 4096;
  char buf[buflen];

  // loop over calls to `getdents64`
  while (1) {
    ssize_t bytes_read = getdents64(dirfd, buf, buflen);
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
          int dirfd2 = openat(dirfd, ent->d_name, O_RDONLY | O_DIRECTORY);
          if (dirfd2 < 0) {
            continue;
          }
          searchdir(dirfd2, topfiles);
          break;
        case DT_REG:
          struct stat statbuf;
          int r = fstatat(dirfd, ent->d_name, &statbuf, 0);
          if (r < 0) {
            continue;
          }

          if (statbuf.st_size > topfiles_list_max_size(topfiles)) {
            struct topfiles_entry topfile = { .name = strdup(ent->d_name), .size = statbuf.st_size };
            topfiles_list_insert_sorted(topfiles, topfile);
          }

          break;
        default:
          continue;
      }
    }
  }
}


int main(int argc, char* argv[]) {
  if (!cs644_check_n_args(1, argc, argv)) {
    fprintf(stderr, "usage: %s DIR\n", argv[0]);
    return 1;
  }

  struct topfiles_list lst = topfiles_list_create(10);
  int dirfd = BAIL_IF_ERR(open(argv[1], O_RDONLY | O_DIRECTORY));
  searchdir(dirfd, &lst);

  for (size_t i = 0; i < lst.len; i++) {
    struct topfiles_entry ent = lst.entries[i];
    printf("%2ld. %s (%ld)\n", i + 1, ent.name, ent.size);
  }

  return 0;
}
