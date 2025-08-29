#include <fcntl.h>
#include <semaphore.h>
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include "cs644.h"

const char* MEM = "/ringbuffer-mem";
const char* SEM = "/ringbuffer-sem";

/**
 * Ring buffer representation:
 *
 *   - 256 bytes total
 *     - [data0, data1, ..., data253, read, write]
 *   - `read` is the next index to be read.
 *   - `write` is the next index to be written.
 *   - If `read == write`, the buffer is empty and reads block.
 *   - If `(write + 1) % N == read`, the buffer is full and writes block.
 *     - This means the buffer can only hold a max of N-1 elements.
 *
 * We use a flat array of bytes; if we used a struct, we'd be dependent on how the
 * compiler decided to lay out the struct in memory.
 *
 * If the payload were more than 256 bytes, we'd have to use more than one byte each
 * for the read and write indices, which makes things a little more complicated (little-
 * endian vs. big-endian, e.g.).
 */

#define BUFFER_PAYLOAD_SIZE 254
#define BUFFER_TOTAL_SIZE (BUFFER_PAYLOAD_SIZE + 2)
#define FIELD_READ_INDEX (BUFFER_TOTAL_SIZE - 2)
#define FIELD_WRITE_INDEX (BUFFER_TOTAL_SIZE - 1)

struct ringbuf {
  sem_t* sem;
  unsigned char* data;
};

struct ringbuf ring_buffer_create() {
  int fd = shm_open(MEM, O_CREAT | O_EXCL | O_RDWR, 0600);
  cs644_bail_if_err(fd, "shm_open");
  int r = ftruncate(fd, BUFFER_TOTAL_SIZE);
  cs644_bail_if_err(r, "ftruncate");
  unsigned char* data = mmap(NULL, BUFFER_TOTAL_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
  if (data == MAP_FAILED) {
    cs644_bail("mmap failed");
  }
  r = close(fd);
  cs644_bail_if_err(r, "close");

  sem_t* sem = sem_open(SEM, O_CREAT | O_EXCL, 0600, 1);
  if (sem == SEM_FAILED) {
    cs644_bail("sem_open failed");
  }

  return (struct ringbuf){ .sem = sem, .data = data };
}

struct ringbuf ring_buffer_open() {
  int fd = shm_open(MEM, O_RDWR, 0);
  cs644_bail_if_err(fd, "shm_open");
  unsigned char* data = mmap(NULL, BUFFER_TOTAL_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
  if (data == MAP_FAILED) {
    cs644_bail("mmap failed");
  }
  int r = close(fd);
  cs644_bail_if_err(r, "close");

  sem_t* sem = sem_open(SEM, 0);
  if (sem == SEM_FAILED) {
    cs644_bail("sem_open failed");
  }

  return (struct ringbuf){ .sem = sem, .data = data };
}

void ring_buffer_destroy(struct ringbuf buf) {
  int r = shm_unlink(MEM);
  cs644_bail_if_err(r, "shm_unlink");

  r = sem_unlink(SEM);
  cs644_bail_if_err(r, "sem_unlink");
}

void ring_buffer_lock(struct ringbuf buf) {
  int r = sem_wait(buf.sem);
  cs644_bail_if_err(r, "sem_wait");
}

void ring_buffer_unlock(struct ringbuf buf) {
  int r = sem_post(buf.sem);
  cs644_bail_if_err(r, "sem_post");
}

unsigned char ring_buffer_acquire_read(struct ringbuf buf) {
  unsigned char ri, wi;
  while (1) {
    ring_buffer_lock(buf);
    ri = buf.data[FIELD_READ_INDEX];
    wi = buf.data[FIELD_WRITE_INDEX];
    if (ri == wi) {
      // must release the lock to give writers a chance to make progress
      ring_buffer_unlock(buf);
      continue;
    } else {
      return ri;
    }
  }
}

unsigned char ring_buffer_acquire_write(struct ringbuf buf) {
  unsigned char ri, wi;
  while (1) {
    ring_buffer_lock(buf);
    ri = buf.data[FIELD_READ_INDEX];
    wi = buf.data[FIELD_WRITE_INDEX];
    if ((wi + 1) % BUFFER_PAYLOAD_SIZE == ri) {
      // must release the lock to give readers a chance to make progress
      ring_buffer_unlock(buf);
      continue;
    } else {
      return wi;
    }
  }
}

void ring_buffer_incr(struct ringbuf buf, unsigned char field) {
  buf.data[field] = (buf.data[field] + 1) % BUFFER_PAYLOAD_SIZE;
}

unsigned char ring_buffer_read(struct ringbuf buf) {
  unsigned char i = ring_buffer_acquire_read(buf);
  unsigned char x = buf.data[i];
  ring_buffer_incr(buf, FIELD_READ_INDEX);
  ring_buffer_unlock(buf);
  return x;
}

void ring_buffer_write(struct ringbuf buf, unsigned char x) {
  unsigned char i = ring_buffer_acquire_write(buf);
  buf.data[i] = x;
  ring_buffer_incr(buf, FIELD_WRITE_INDEX);
  ring_buffer_unlock(buf);
}

void producer() {
  struct ringbuf buf = ring_buffer_create();
  puts("Press enter to start");
  getchar();

  for (int i = 0; i < 2; i++) {
    for (int j = 0; j < BUFFER_PAYLOAD_SIZE; j++) {
      ring_buffer_write(buf, j);
    }
  }
  ring_buffer_destroy(buf);
}

void consumer() {
  struct ringbuf buf = ring_buffer_open();
  for (;;) {
    unsigned char x = ring_buffer_read(buf);
    printf("read: %d\n", x);
  }
}

int main(int argc, char* argv[]) {
  if (argc != 2) {
    return 1;
  }

  if (strcmp(argv[1], "producer") == 0) {
    producer();
  } else if (strcmp(argv[1], "consumer") == 0) {
    consumer();
  } else {
    return 1;
  }
}
