#define _GNU_SOURCE
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <stdio.h>
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

void do_one_getdents() {
}

void* test_atomic_getdents(void* data) {
  size_t n = 50000;
  char buf[n];

  int fd = open(TMPDIR, O_RDONLY | O_DIRECTORY);
  handle_err(fd, "open");

  for (int i = 0; i < 200; i++) {
    ssize_t bytes = getdents64(fd, buf, n);
    handle_err(bytes, "getdents64");

    struct linux_dirent* ent;
    int seen[26] = {0};
    for (size_t bpos = 0; bpos < bytes;) {
      ent = (struct linux_dirent*)(buf + bpos);
      char c = ent->d_name[0];
      if (c >= 'a' && c <= 'z') {
        int i = c - 'a';
        if (seen[i]) {
          printf("getdents: saw both old and new versions of '%c'\n", c);
          pthread_exit(NULL);
        }
        seen[i] = 1;
      }
      bpos += ent->d_reclen;
    }
  }

  int r = close(fd);
  handle_err(r, "close");

  return NULL;
}

void do_one_readdir() {
  DIR* dp = opendir(TMPDIR);
  struct dirent* ent;

  int seen[26] = {0};
  while ((ent = readdir(dp)) != NULL) {
    char c = ent->d_name[0];
    if (!(c >= 'a' && c <= 'z')) {
      continue;
    }

    int i = c - 'a';
    if (seen[i]) {
      printf("readdir: saw both old and new versions of '%c'\n", c);
      pthread_exit(NULL);
    }
    seen[i] = 1;
  }
}

void* test_atomic_readdir(void* data) {
  for (int i = 0; i < 200; i++) {
    do_one_readdir();
  }

  return NULL;
}

void shuffle_entries(char c) {
  char old[] = { c, '\0' };
  char new[] = { c, '+', '\0' };

  unlink(new);
  int fd = open(old, O_WRONLY | O_CREAT | O_TRUNC, 0600);
  handle_err(fd, "open");
  int r = close(fd);
  handle_err(fd, "close");

  int i = 0;
  while (1) {
    r = rename(old, new);
    if (r < 0) {
      printf("rename (old -> new): %s (e=%s, c='%c', i=%d)\n",
          strerror(errno), strerrorname_np(errno), c, i);
      break;
    }

    r = rename(new, old);
    if (r < 0) {
      printf("rename (new -> old): %s (e=%s, c='%c', i=%d)\n",
          strerror(errno), strerrorname_np(errno), c, i);
      break;
    }
    i++;
  }
}

void* shuffle_entries_thrd(void* data) {
  shuffle_entries(*((char*)data));
  return NULL;
}

int main() {
  int r = mkdir(TMPDIR, 0700);
  if (r < 0 && errno != EEXIST) {
    handle_err(r, "mkdir");
  }
  r = chdir(TMPDIR);
  handle_err(r, "chdir");

  pthread_t tid1;
  pthread_create(&tid1, NULL, test_atomic_getdents, NULL);
  pthread_t tid2;
  pthread_create(&tid2, NULL, test_atomic_readdir, NULL);

  char* letters = "abcdefghijklmnopqrstuvwxyz";
  for (char* p = letters; *p; p++) {
    pthread_t tid;
    pthread_create(&tid, NULL, shuffle_entries_thrd, p);
  }

  //pthread_join(tid1, NULL);
  pthread_join(tid2, NULL);
  return 0;
}
