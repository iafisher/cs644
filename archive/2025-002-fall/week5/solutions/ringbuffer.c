/**
 * https://iafisher.com/cs644/fall2025/week5
 */
#include <assert.h>
#include <fcntl.h>
#include <semaphore.h>
#include <stdalign.h>
#include <stdint.h>
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
 *   - 4096 bytes total (same as system memory page size)
 *     - [data0, data1, ..., data4091, read, write]
 *   - `read` is the next index to be read.
 *   - `write` is the next index to be written.
 *   - If `read == write`, the buffer is empty and reads block.
 *   - If `(write + 1) % N == read`, the buffer is full and writes block.
 *     - This means the buffer can only hold a max of N-1 elements.
 */

#define BUFFER_PAYLOAD_SIZE 4092

// This is the actual data structure that is put in shared memory.
struct ringbuf_data {
  // Explicitly set alignment to ensure consistent in-memory representation
  alignas(16) unsigned char data[BUFFER_PAYLOAD_SIZE];
  uint16_t read_index;
  uint16_t write_index;
};

static_assert(sizeof(struct ringbuf_data) == 4096, "layout changed");

// This is a container that wraps the semaphore and the shared memory object.
//
// It is not itself mapped to shared memory.
struct ringbuf {
  sem_t* sem;
  struct ringbuf_data* data;
};

struct ringbuf ring_buffer_create() {
  int fd = BAIL_IF_ERR(shm_open(MEM, O_CREAT | O_EXCL | O_RDWR, 0600));
  BAIL_IF_ERR(ftruncate(fd, sizeof(struct ringbuf_data)));
  // `ftruncate` will fill the region with null bytes, so `read_index` and `write_index` are
  // initially set to 0.
  unsigned char* data = mmap(NULL, sizeof(struct ringbuf_data), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
  if (data == MAP_FAILED) {
    cs644_bail("mmap failed");
  }
  BAIL_IF_ERR(close(fd));

  sem_t* sem = sem_open(SEM, O_CREAT | O_EXCL, 0600, 1);
  if (sem == SEM_FAILED) {
    cs644_bail("sem_open failed");
  }

  return (struct ringbuf){ .sem = sem, .data = (struct ringbuf_data*)data };
}

struct ringbuf ring_buffer_open() {
  int fd = BAIL_IF_ERR(shm_open(MEM, O_RDWR, 0));
  unsigned char* data = mmap(NULL, sizeof(struct ringbuf_data), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
  if (data == MAP_FAILED) {
    cs644_bail("mmap failed");
  }
  BAIL_IF_ERR(close(fd));

  sem_t* sem = sem_open(SEM, 0);
  if (sem == SEM_FAILED) {
    cs644_bail("sem_open failed");
  }

  return (struct ringbuf){ .sem = sem, .data = (struct ringbuf_data*)data };
}

void ring_buffer_destroy(struct ringbuf buf) {
  BAIL_IF_ERR(shm_unlink(MEM));
  BAIL_IF_ERR(sem_unlink(SEM));
}

void ring_buffer_lock(struct ringbuf buf) {
  BAIL_IF_ERR(sem_wait(buf.sem));
}

void ring_buffer_unlock(struct ringbuf buf) {
  BAIL_IF_ERR(sem_post(buf.sem));
}

// NOTE: This implementation "busy-waits", which is simple but not very efficient.

uint16_t ring_buffer_acquire_read(struct ringbuf buf) {
  uint16_t ri, wi;
  while (1) {
    ring_buffer_lock(buf);
    ri = buf.data->read_index;
    wi = buf.data->write_index;
    if (ri == wi) {
      // must release the lock to give writers a chance to make progress
      ring_buffer_unlock(buf);
      continue;
    } else {
      return ri;
    }
  }
}

uint16_t ring_buffer_acquire_write(struct ringbuf buf) {
  uint16_t ri, wi;
  while (1) {
    ring_buffer_lock(buf);
    ri = buf.data->read_index;
    wi = buf.data->write_index;
    if ((wi + 1) % BUFFER_PAYLOAD_SIZE == ri) {
      // must release the lock to give readers a chance to make progress
      ring_buffer_unlock(buf);
      continue;
    } else {
      return wi;
    }
  }
}

unsigned char ring_buffer_read(struct ringbuf buf) {
  uint16_t i = ring_buffer_acquire_read(buf);
  unsigned char x = buf.data->data[i];
  buf.data->read_index = (buf.data->read_index + 1) % BUFFER_PAYLOAD_SIZE;
  ring_buffer_unlock(buf);
  return x;
}

void ring_buffer_write(struct ringbuf buf, unsigned char x) {
  uint16_t i = ring_buffer_acquire_write(buf);
  buf.data->data[i] = x;
  buf.data->write_index = (buf.data->write_index + 1) % BUFFER_PAYLOAD_SIZE;
  ring_buffer_unlock(buf);
}

void producer() {
  struct ringbuf buf = ring_buffer_create();
  puts("Press <Enter> to start");
  getchar();

  for (int i = 0; i < 2; i++) {
    for (int j = 0; j < BUFFER_PAYLOAD_SIZE; j++) {
      ring_buffer_write(buf, j);
    }
  }

  puts("Press <Enter> to quit");
  getchar();
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
    cs644_bail("expected first argument to be 'producer' or 'consumer'");
  }

  return 0;
}
