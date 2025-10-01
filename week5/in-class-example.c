#include <fcntl.h>
#include <stdalign.h>
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>
#include "cs644.h"

const char* mem_pathname = "/my-program-mem";
const char* sem_pathname = "/my-program-sem";

struct my_counters {
  alignas(4) int counter1;
  int counter2;
};

void writer() {
  int fd = BAIL_IF_ERR(shm_open(mem_pathname, O_CREAT | O_EXCL | O_RDWR, 0600));
  BAIL_IF_ERR(ftruncate(fd, sizeof(struct my_counters)));

  struct my_counters* s;
  s = mmap(NULL, sizeof *s, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
  if (s == MAP_FAILED) { cs644_bail("mmap"); }
  s->counter1 = 0;
  s->counter2 = 0;

  for (;;) {
    puts("Press <Enter> to increment counter 1");
    getchar();
    s->counter1++;
    puts("Press <Enter> to increment counter 2");
    getchar();
    s->counter2++;
  }
}

void reader() {
  int fd = BAIL_IF_ERR(shm_open(mem_pathname, O_RDONLY, 0));

  struct my_counters* s;
  s = mmap(NULL, sizeof *s, PROT_READ, MAP_SHARED, fd, 0);
  if (s == MAP_FAILED) { cs644_bail("mmap"); }

  for (;;) {
    puts("Press <Enter> to read counter values");
    getchar();
    printf("Counter 1: %d\n", s->counter1);
    printf("Counter 2: %d\n", s->counter2);
  }
}

int main(int argc, char* argv[]) {
  if (argc != 2) {
    return 1;
  }

  if (strcmp(argv[1], "reader") == 0) {
    reader();
  } else if (strcmp(argv[1], "writer") == 0) {
    writer();
  } else {
    return 1;
  }
}
