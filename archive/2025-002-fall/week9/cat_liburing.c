/**
 * Adapted from https://unixism.net/loti/tutorial/cat_liburing.html
 */
#include <liburing.h>
#include <linux/fs.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include "cs644.h"

#define QUEUE_DEPTH      1
#define MY_BLOCK_SIZE 1024

struct file_info {
  off_t file_sz;
  struct iovec iovecs[];
};

off_t get_file_size(int fd);
void output_to_console(char* buf, int len);

int get_completion_and_print(struct io_uring* ring) {
  struct io_uring_cqe* cqe;
  BAIL_IF_ERR(io_uring_wait_cqe(ring, &cqe));
  if (cqe->res < 0) {
    fputs("error: async readv failed\n", stderr);
    return 1;
  }

  struct file_info* fi = io_uring_cqe_get_data(cqe);
  int blocks = (int)fi->file_sz / MY_BLOCK_SIZE;
  if (fi->file_sz % MY_BLOCK_SIZE) {
    blocks++;
  }

  for (int i = 0; i < blocks; i++) {
    output_to_console(fi->iovecs[i].iov_base, fi->iovecs[i].iov_len);
  }

  io_uring_cqe_seen(ring, cqe);
  return 0;
}

int submit_read_request(char* file_path, struct io_uring* ring) {
  int fd = BAIL_IF_ERR(open(file_path, O_RDONLY));
  off_t file_sz = get_file_size(fd);
  int blocks = (int)file_sz / MY_BLOCK_SIZE;
  if (file_sz % MY_BLOCK_SIZE) {
    blocks++;
  }

  struct file_info* fi = cs644_malloc_or_bail(sizeof *fi + sizeof(struct iovec) * blocks);

  off_t bytes_remaining = file_sz;
  off_t offset = 0;
  int current_block = 0;
  while (bytes_remaining) {
    off_t bytes_to_read = bytes_remaining;
    if (bytes_to_read > MY_BLOCK_SIZE) {
      bytes_to_read = MY_BLOCK_SIZE;
    }

    offset += bytes_to_read;
    fi->iovecs[current_block].iov_len = bytes_to_read;
    void* buf;
    BAIL_IF_ERR(posix_memalign(&buf, MY_BLOCK_SIZE, MY_BLOCK_SIZE));
    fi->iovecs[current_block].iov_base = buf;

    current_block++;
    bytes_remaining -= bytes_to_read;
  }
  fi->file_sz = file_sz;

  struct io_uring_sqe* sqe = io_uring_get_sqe(ring);
  io_uring_prep_readv(sqe, fd, fi->iovecs, blocks, 0);
  io_uring_sqe_set_data(sqe, fi);
  io_uring_submit(ring);
  return 0;
}

int main(int argc, char* argv[]) {
  struct io_uring ring;

  io_uring_queue_init(QUEUE_DEPTH, &ring, 0);

  for (int i = 1; i < argc; i++) {
    int r = submit_read_request(argv[i], &ring);
    if (r < 0) {
      fprintf(stderr, "error: could not read file: %s\n", argv[i]);
      return 1;
    }
    get_completion_and_print(&ring);
  }

  io_uring_queue_exit(&ring);
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
