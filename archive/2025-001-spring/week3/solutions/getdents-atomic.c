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

const char* TMPDIR = "/tmp/getdents-test";

struct linux_dirent {
  unsigned long  d_ino;
  off_t          d_off;
  unsigned short d_reclen;
  char           d_name[];
};

void* test_atomic_getdents(void* data) {
  size_t n = 50000;
  char buf[n];

  int fd = open(TMPDIR, O_RDONLY | O_DIRECTORY);
  cs644_bail_if_err(fd, "open");

  for (int i = 0; i < 200; i++) {
    if ((i + 1) % 10 == 0) {
      printf("getdents: iter %d\n", i);
    }

    ssize_t bytes = getdents64(fd, buf, n);
    cs644_bail_if_err(bytes, "getdents64");

    struct linux_dirent* ent;
    int seen[1000] = {0};
    for (size_t bpos = 0; bpos < bytes;) {
      ent = (struct linux_dirent*)(buf + bpos);
      int i = atoi(ent->d_name);
      if (i != 0) {
        if (i > 1000) {
          i -= 1000;
        }

        if (seen[i]) {
          printf("getdents: saw both old and new versions of '%s'\n", ent->d_name);
          pthread_exit(NULL);
        }
        seen[i] = 1;
      }
      bpos += ent->d_reclen;
    }
  }

  int r = close(fd);
  cs644_bail_if_err(r, "close");

  return NULL;
}

void do_one_readdir() {
  DIR* dp = opendir(TMPDIR);
  struct dirent* ent;

  int seen[1000] = {0};
  while ((ent = readdir(dp)) != NULL) {
    int i = atoi(ent->d_name);
    if (i == 0) {
      continue;
    }

    if (i >= 1000) {
      i -= 1000;
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
    if ((i + 1) % 10 == 0) {
      printf("readdir: iter %d\n", i);
    }

    do_one_readdir();
  }

  return NULL;
}

void shuffle_entries(char* old) {
  char new[100];
  snprintf(new, 100, "%04d", atoi(old) + 1000);

  unlink(new);
  int fd = open(old, O_WRONLY | O_CREAT | O_TRUNC, 0600);
  cs644_bail_if_err(fd, "open");
  int r = close(fd);
  cs644_bail_if_err(fd, "close");

  int i = 0;
  while (1) {
    r = rename(old, new);
    if (r < 0) {
      printf("rename (old -> new): %s (e=%s, s='%s', i=%d)\n",
          strerror(errno), strerrorname_np(errno), old, i);
      break;
    }

    r = rename(new, old);
    if (r < 0) {
      printf("rename (new -> old): %s (e=%s, s='%s', i=%d)\n",
          strerror(errno), strerrorname_np(errno), old, i);
      break;
    }
    i++;
  }
}

void* shuffle_entries_thrd(void* data) {
  shuffle_entries(data);
  return NULL;
}

int main() {
  int r = mkdir(TMPDIR, 0700);
  if (r < 0 && errno != EEXIST) {
    cs644_bail_if_err(r, "mkdir");
  }
  r = chdir(TMPDIR);
  cs644_bail_if_err(r, "chdir");

  pthread_t tid1;
  pthread_create(&tid1, NULL, test_atomic_getdents, NULL);
  pthread_t tid2;
  pthread_create(&tid2, NULL, test_atomic_readdir, NULL);

  for (int i = 1; i < 1000; i++) {
    size_t bufsz = 5;
    char* buf = cs644_malloc_or_bail(bufsz);
    snprintf(buf, bufsz, "%04d", i);
    pthread_t tid;
    pthread_create(&tid, NULL, shuffle_entries_thrd, buf);
  }

  pthread_join(tid1, NULL);
  pthread_join(tid2, NULL);
  return 0;
}
