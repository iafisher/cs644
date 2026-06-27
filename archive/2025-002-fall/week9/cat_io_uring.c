/**
 * Adapted from https://unixism.net/loti/low_level.html
 *
 * (which in turn seems to be heavily inspired by io_uring(7))
 */

#include <fcntl.h>
#include <linux/fs.h>
#include <linux/io_uring.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <sys/uio.h>
#include <unistd.h>
#include "cs644.h"

#define QUEUE_DEPTH     1
#define MY_BLOCK_SIZE   1024

// compiler memory fences preventing instruction re-ordering
#define read_barrier() __asm__ __volatile__("":::"memory")
#define write_barrier() __asm__ __volatile__("":::"memory")

/**
 * Helper data structures for accessing the submission and completion queues.
 *
 * Each field is a pointer into a region of memory shared between userspace and kernelspace.
 */

struct app_io_sq_ring {
  // head and tail are indices into the array field
  unsigned int* head;
  unsigned int* tail;
  // Invariant: ring_mask = ring_entries - 1
  //
  // ring_entries is the total size of the ring buffer, while ring_mask lets you efficiently
  // calculate index % ring_entries, as index & ring_mask is equal (assuming ring_entries is a
  // power of 2) but much more efficient.
  unsigned int* ring_mask;
  unsigned int* ring_entries;
  unsigned int* flags;
  // The submission queue is a ring buffer of indices.
  unsigned int* array;
};

struct app_io_cq_ring {
  unsigned int* head;
  unsigned int* tail;
  unsigned int* ring_mask;
  unsigned int* ring_entries;
  // The completion queue is a ring buffer of completion queue entries.
  struct io_uring_cqe* cqes;
};

struct submitter {
  int ring_fd;
  struct app_io_sq_ring sq_ring;
  // The actual array of submission queue entries (separate from the ring buffer of indices).
  struct io_uring_sqe* sqes;
  struct app_io_cq_ring cq_ring;
};

struct file_info {
  off_t file_sz;
  struct iovec iovecs[];
};

off_t get_file_size(int fd);
void output_to_console(char* buf, int len);

/**
 * Wrapper functions around the raw io_uring syscalls
 */

int io_uring_setup(unsigned entries, struct io_uring_params *p) {
    return (int) syscall(__NR_io_uring_setup, entries, p);
}

int io_uring_enter(int ring_fd, unsigned int to_submit, unsigned int min_complete, unsigned int flags) {
    return (int) syscall(__NR_io_uring_enter, ring_fd, to_submit, min_complete, flags, NULL, 0);
}

int app_setup_uring(struct submitter* submitter) {
  struct app_io_sq_ring* sring = &submitter->sq_ring;
  struct app_io_cq_ring* cring = &submitter->cq_ring;
  struct io_uring_params p;

  memset(&p, 0, sizeof p);
  submitter->ring_fd = BAIL_IF_ERR(io_uring_setup(QUEUE_DEPTH, &p));

  // The total size of each queue is the size of the queue itself, plus an offset to account for
  // the data structure header.
  int sring_sz = p.sq_off.array + p.sq_entries * sizeof(unsigned int);
  int cring_sz = p.cq_off.cqes + p.cq_entries * sizeof(struct io_uring_cqe);
  if (cring_sz > sring_sz) {
    sring_sz = cring_sz;
  }
  cring_sz = sring_sz;
  
  // Logically, we should need 2 calls to mmap, one for the submission queue and one for the
  // completion queue. For efficiency, the kernel has special support for mapping both in with
  // 1 call.
  //
  // Why is it max(sring_sz, cring_sz) and not sring_sz + cring_sz? sring_sz and cring_sz were
  // populated from the sq_off and cq_off structs, which already account for the shared allocation.
  void* sq_ptr = mmap(
      0, sring_sz, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_POPULATE, submitter->ring_fd, IORING_OFF_SQ_RING);
  if (sq_ptr == MAP_FAILED) {
    perror("mmap (sq_ptr)");
    return 1;
  }

  void* cq_ptr = sq_ptr;

  // fill in our helper data structure
  sring->head = sq_ptr + p.sq_off.head;
  sring->tail = sq_ptr + p.sq_off.tail;
  sring->ring_mask = sq_ptr + p.sq_off.ring_mask;
  sring->ring_entries = sq_ptr + p.sq_off.ring_entries;
  sring->flags = sq_ptr + p.sq_off.flags;
  sring->array = sq_ptr + p.sq_off.array;

  submitter->sqes = mmap(
      0, p.sq_entries * sizeof(struct io_uring_sqe), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_POPULATE,
      submitter->ring_fd, IORING_OFF_SQES);
  if (submitter->sqes == MAP_FAILED) {
    perror("mmap (submitter->sqes)");
    return 1;
  }

  // fill in our helper data structure
  cring->head = cq_ptr + p.cq_off.head;
  cring->tail = cq_ptr + p.cq_off.tail;
  cring->ring_mask = cq_ptr + p.cq_off.ring_mask;
  cring->ring_entries = cq_ptr + p.cq_off.ring_entries;
  cring->cqes = cq_ptr + p.cq_off.cqes;

  return 0;
}

void read_from_cq(struct submitter* submitter) {
  struct app_io_cq_ring* cring = &submitter->cq_ring;
  unsigned int head = *cring->head;

  do {
    read_barrier();
    if (head == *cring->tail) {
      break;
    }

    struct io_uring_cqe* cqe = &cring->cqes[head & *submitter->cq_ring.ring_mask];
    struct file_info* fi = (struct file_info*)cqe->user_data;
    if (cqe->res < 0) {
      fprintf(stderr, "error: %s\n", strerror(abs(cqe->res)));
      continue;
    }

    // figure out how many iovec blocks we need to read
    int blocks = (int)fi->file_sz / MY_BLOCK_SIZE;
    if (fi->file_sz % MY_BLOCK_SIZE) {
      // if fi->file_sz isn't even divisible by MY_BLOCK_SIZE, we need one extra block
      blocks++;
    }

    for (int i = 0; i < blocks; i++) {
      output_to_console(fi->iovecs[i].iov_base, fi->iovecs[i].iov_len);
    }

    head++;
  } while (1);

  *cring->head = head;
  write_barrier();
}

int submit_to_sq(char* file_path, struct submitter* submitter) {
  int fd = BAIL_IF_ERR(open(file_path, O_RDONLY));
  struct app_io_sq_ring* sring = &submitter->sq_ring;

  off_t file_sz = get_file_size(fd);
  if (file_sz < 0) {
    return 1;
  }

  off_t bytes_remaining = file_sz;
  int blocks = (int)file_sz / MY_BLOCK_SIZE;
  if (file_sz % MY_BLOCK_SIZE) {
    blocks++;
  }

  struct file_info* fi = cs644_malloc_or_bail(sizeof *fi + sizeof(struct iovec) * blocks);
  fi->file_sz = file_sz;

  unsigned int current_block = 0;
  while (bytes_remaining) {
    off_t bytes_to_read = bytes_remaining;
    if (bytes_to_read > MY_BLOCK_SIZE) {
      bytes_to_read = MY_BLOCK_SIZE;
    }

    fi->iovecs[current_block].iov_len = bytes_to_read;

    void* buf;
    BAIL_IF_ERR(posix_memalign(&buf, MY_BLOCK_SIZE, MY_BLOCK_SIZE));
    fi->iovecs[current_block].iov_base = buf;

    current_block++;
    bytes_remaining -= bytes_to_read;
  }

  unsigned int next_tail, tail;
  next_tail = tail = *sring->tail;
  next_tail++;
  read_barrier();
  // get the index into the SQE array
  unsigned int index = tail & *submitter->sq_ring.ring_mask;
  // fill in the readv request
  struct io_uring_sqe* sqe = &submitter->sqes[index];
  sqe->fd = fd;
  sqe->flags = 0;
  sqe->opcode = IORING_OP_READV;
  sqe->addr = (unsigned long)fi->iovecs;
  sqe->len = blocks;
  sqe->off = 0;
  sqe->user_data = (unsigned long long)fi;
  sring->array[index] = index;
  tail = next_tail;

  if (*sring->tail != tail) {
    *sring->tail = tail;
    write_barrier();
  }

  BAIL_IF_ERR(io_uring_enter(submitter->ring_fd, 1, 1, IORING_ENTER_GETEVENTS));
  return 0;
}

int main(int argc, char* argv[]) {
  struct submitter* submitter = cs644_malloc_or_bail(sizeof *submitter);
  memset(submitter, 0, sizeof *submitter);

  if (app_setup_uring(submitter)) {
    fputs("error: unable to set up uring\n", stderr);
    return 1;
  }

  for (int i = 1; i < argc; i++) {
    if (submit_to_sq(argv[i], submitter)) {
      fprintf(stderr, "error: could not read file: %s\n", argv[i]);
      return 1;
    }
    read_from_cq(submitter);
  }

  return 0;
}

off_t get_file_size(int fd) {
  struct stat statbuf;
  BAIL_IF_ERR(fstat(fd, &statbuf));
  if (S_ISBLK(statbuf.st_mode)) {
    unsigned long long bytes;
    BAIL_IF_ERR(ioctl(fd, BLKGETSIZE64, &bytes));
    return bytes;
  } else if (S_ISREG(statbuf.st_mode)) {
    return statbuf.st_size;
  } else {
    return -1;
  }
}

void output_to_console(char* buf, int len) {
  while (len--) {
    fputc(*buf++, stdout);
  }
}
