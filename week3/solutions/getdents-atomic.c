/**
 * https://iafisher.com/cs644/fall2025/week3
 *
 * Is getdents64 atomic?
 *
 * Idea: Create a directory with 1,000 files named 0000 through 0999. With one thread per file,
 * rename X to X+1000 (e.g., 0000 becomes 1000) and back. If getdents64 presents a consistent view
 * of the directory, we should never see both X and X+1000 simultaneously in the same call.
 */
#define _GNU_SOURCE
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>
#include "cs644.h"

#define NFILES            10000
#define WIDTH                 5
#define FMTSTR           "%05d"
#define FILES_PER_THREAD    100

const char* TMPDIR = "/tmp/getdents-test";

struct linux_dirent64 {
  ino64_t        d_ino;
  off64_t        d_off;
  unsigned short d_reclen;
  unsigned char  d_type;
  char           d_name[];
};

void* test_atomic_getdents(void* data) {
  size_t n = 50000;
  char buf[n];

  int fd = open(TMPDIR, O_RDONLY | O_DIRECTORY);
  cs644_bail_if_err(fd, "open");

  for (int i = 0; i < 200; i++) {
    ssize_t bytes = getdents64(fd, buf, n);
    cs644_bail_if_err(bytes, "getdents64");

    struct linux_dirent64* ent;
    int seen[NFILES] = {0};
    for (size_t bpos = 0; bpos < bytes;) {
      ent = (struct linux_dirent64*)(buf + bpos);
      int i = atoi(ent->d_name);
      if (i != 0) {
        if (i > NFILES) {
          i -= NFILES;
        }

        if (seen[i]) {
          printf("getdents: saw both old and new versions of '%s'\n", ent->d_name);
          pthread_exit(NULL);
        }
        seen[i] = 1;
      }
      bpos += ent->d_reclen;
    }

    if ((i + 1) % 10 == 0) {
      printf("getdents: ran %d iterations\n", i + 1);
    }
  }

  int r = close(fd);
  cs644_bail_if_err(r, "close");

  return NULL;
}

void do_one_readdir() {
  DIR* dp = opendir(TMPDIR);
  struct dirent* ent;

  int seen[NFILES] = {0};
  while ((ent = readdir(dp)) != NULL) {
    int i = atoi(ent->d_name);
    if (i == 0) {
      continue;
    }

    if (i >= NFILES) {
      i -= NFILES;
    }

    if (seen[i]) {
      printf("readdir: saw both old and new versions of '%s'\n", ent->d_name);
      pthread_exit(NULL);
    }
    seen[i] = 1;
  }
}

void* test_atomic_readdir(void* data) {
  for (int i = 0; i < 200; i++) {
    do_one_readdir();

    if ((i + 1) % 10 == 0) {
      printf("readdir: ran %d iterations\n", i + 1);
    }
  }

  return NULL;
}

void shuffle_entries(int first) {
  // loop forever, moving `old` to `new` and back again for each file in the batch
  unsigned long long loop_count = 0;
  while (1) {
    for (int i = first; i < first + FILES_PER_THREAD; i++) {
      char old[100];
      snprintf(old, 100, FMTSTR, i);

      char new[100];
      snprintf(new, 100, FMTSTR, i + NFILES);

      // ensure that `old` (e.g., 00001) exists and `new` (e.g., 10001) does not
      unlink(new);
      int fd = open(old, O_WRONLY | O_CREAT | O_TRUNC, 0600);
      cs644_bail_if_err(fd, "open");
      int r = close(fd);
      cs644_bail_if_err(fd, "close");

      r = rename(old, new);
      if (r < 0) {
        printf("rename (old -> new): %s (e=%s, s='%s', loop_count=%lld)\n",
            strerror(errno), strerrorname_np(errno), old, loop_count);
        break;
      }

      r = rename(new, old);
      if (r < 0) {
        printf("rename (new -> old): %s (e=%s, s='%s', loop_count=%lld)\n",
            strerror(errno), strerrorname_np(errno), old, loop_count);
        break;
      }
    }

    loop_count++;
  }
}

void* shuffle_entries_thrd(void* data) {
  long long first = (long long)data;
  shuffle_entries(first);
  return NULL;
}

void usage(char* progname) {
  fprintf(stderr, "usage: %s <getdents|readdir>\n", progname);
  exit(1);
}

int main(int argc, char* argv[]) {
  if (argc != 2) {
    usage(argv[0]);
  }

  int use_getdents = 0;
  if (strcmp(argv[1], "getdents") == 0 || strcmp(argv[1], "getdents64") == 0) {
    use_getdents = 1;
  } else if (strcmp(argv[1], "readdir") != 0) {
    usage(argv[0]);
  }

  int r = mkdir(TMPDIR, 0700);
  if (r < 0 && errno != EEXIST) {
    cs644_bail_if_err(r, "mkdir");
  }
  r = chdir(TMPDIR);
  cs644_bail_if_err(r, "chdir");

  // Here we use pthreads to call two functions simultaneously. We'll learn more about this in
  // future weeks.
  pthread_t maintid;
  pthread_create(&maintid, NULL, use_getdents ? test_atomic_getdents : test_atomic_readdir, NULL);

  // call `shuffle_entries("00001")`, `shuffle_entries("00002")`, etc.
  for (long long i = 1; i < NFILES; i += FILES_PER_THREAD) {
    pthread_t tid;
    pthread_create(&tid, NULL, shuffle_entries_thrd, (void*)i);
  }

  pthread_join(maintid, NULL);
  return 0;
}
