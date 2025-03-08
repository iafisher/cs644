#include <dirent.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include "cs644.h"

void remove_file(const char* pathname);
void remove_dir(const char* pathname);

void remove_file_or_dir(const char* pathname) {
  struct stat statbuf;
  int r = stat(pathname, &statbuf);
  handle_err(r, "stat");

  switch (statbuf.st_mode & S_IFMT) {
    case S_IFLNK:
    case S_IFREG:
      remove_file(pathname);
      return;
    case S_IFDIR:
      remove_dir(pathname);
      return;
    default:
      bail("not removing unknown file type");
  }
}

void remove_file(const char* pathname) {
  int r = unlink(pathname);
  handle_err(r, "unlink");
}

char* make_subpath(const char* parent, const char* child) {
  size_t n1 = strlen(parent);
  size_t n2 = strlen(child);

  char* r = malloc_s((n1 + n2 + 2) * sizeof *r);
  strcpy(r, parent);
  r[n1] = '/';
  strcpy(r + n1 + 1, child);
  return r;
}

void remove_dir(const char* pathname) {
  DIR* dir = opendir(pathname);
  if (dir == NULL) {
    perror("opendir");
    exit(1);
  }

  struct dirent* ent;
  while (1) {
    errno = 0;
    ent = readdir(dir);
    if (ent == NULL) {
      if (errno == 0) {
        break;
      } else {
        perror("readdir");
        exit(1);
      }
    }

    if (strcmp(ent->d_name, "..") == 0 || strcmp(ent->d_name, ".") == 0) {
      continue;
    }

    char* sbp = make_subpath(pathname, ent->d_name);

    switch (ent->d_type) {
      case DT_DIR:
        remove_dir(sbp);
        break;
      case DT_LNK:
      case DT_REG:
        remove_file(sbp);
        break;
      default:
        bail("not removing unknown file type");
    }

    free(sbp);
  }

  int r = rmdir(pathname);
  handle_err(r, "rmdir");
}

int main(int argc, char* argv[]) {
  if (argc != 2 || argv[1][0] == '-') {
    return 1;
  }

  remove_file_or_dir(argv[1]);

  return 0;
}
