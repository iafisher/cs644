#include <fcntl.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

void bail(const char* s) {
  perror(s);
  exit(1);
}

#define SIZE 100

const char* shared_mem_path = "/cs644-example-mem";
const char* shared_sem_path = "/cs644-example-sem";

void writer() {
  // idea: shared memory array of 100 bytes, increment all of them
  // at once
  int fd = shm_open(shared_mem_path, O_CREAT | O_EXCL | O_RDWR, 0600);
  if (fd < 0) { bail("shm_open"); }

  int r = ftruncate(fd, SIZE);
  if (r < 0) { bail("ftruncate"); }

  unsigned char* arr = mmap(NULL, SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
  if (arr == MAP_FAILED) { bail("mmap"); }
  r = close(fd);
  if (r < 0) { bail("close"); }

  sem_t* sem = sem_open(shared_sem_path, O_CREAT | O_EXCL | O_RDWR, 0600, 1);
  if (sem == SEM_FAILED) { bail("sem_open"); }

  for (size_t j = 0; j < 10000000; j++) {
    r = sem_wait(sem);
    if (r < 0) { bail("sem_wait"); }
    puts("writing!");

    for (size_t i = 0; i < SIZE; i++) {
      arr[i] += 1;
    }

    r = sem_post(sem);
    if (r < 0) { bail("sem_post"); }
  }

  r = sem_unlink(shared_sem_path);
  if (r < 0) { bail("sem_unlink"); }

  r = shm_unlink(shared_mem_path);
  if (r < 0) { bail("shm_unlink"); }
}

void reader() {
  int fd = shm_open(shared_mem_path, O_RDONLY, 0);
  if (fd < 0) { bail("shm_open"); }

  unsigned char* arr = mmap(NULL, SIZE, PROT_READ, MAP_SHARED, fd, 0);
  int r = close(fd);
  if (r < 0) { bail("close"); }

  sem_t* sem = sem_open(shared_sem_path, O_RDONLY);
  if (sem == SEM_FAILED) { bail("sem_open"); }

  for (size_t j = 0; j < 100000000; j++) {
    r = sem_wait(sem);
    if (r < 0) { bail("sem_wait"); }
    puts("reading!");

    unsigned char val = arr[0];
    for (size_t i = 1; i < SIZE; i++) {
      if (val != arr[i]) {
        puts("partial write detected!");
        return;
      }
    }

    r = sem_post(sem);
    if (r < 0) { bail("sem_post"); }
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
