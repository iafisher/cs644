/**
 * https://iafisher.com/cs644/fall2025/week3
 *
 * Based on https://danluu.com/file-consistency/
 */
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "cs644.h"

void usage(char* progname) {
  fprintf(stderr, "usage: %s save FILE STR OFFSET\n", progname);
  fprintf(stderr, "       %s corrupt FILE\n", progname);
  fprintf(stderr, "       %s recover FILE\n", progname);
  fprintf(stderr, "       %s read-log LOGFILE\n", progname);
  exit(1);
}

// caller should `free` the result
char* make_logpath(const char* filepath) {
  struct cs644_str s = cs644_str_new();
  cs644_str_append(&s, filepath, strlen(filepath));
  cs644_str_append(&s, ".log", 4);
  return s.data;
}

struct logged_write {
  size_t offset;
  size_t len;
  const char* data; // NULL if no write was logged
};

// NOTE: `read_u64` and `write_u64` are dependent on the machine's byte order.
// This seems like a reasonable assumption since we are writing to a single machine's
// disk, not over the network. Also, almost all machines nowadays are little-endian.

void write_u64(int fd, size_t x) {
  // pointer cast to reinterpret `x` as an array of bytes
  char* as_bytes = (char*)&x;
  ssize_t nwritten = write(fd, as_bytes, sizeof x);
  cs644_bail_if_err(nwritten, "write (u64)");
}

size_t read_u64(int fd) {
  size_t n = sizeof(size_t);
  char buf[n];
  ssize_t nread = read(fd, buf, n);
  cs644_bail_if_err(nread, "read (u64)");
  return *((size_t*)buf);
}

struct logged_write read_log(const char* logpath) {
  int fd = open(logpath, O_RDONLY);
  if (fd < 0) {
    if (errno == ENOENT) {
      return (struct logged_write){ .offset = 0, .len = 0, .data = NULL };
    }
    cs644_bail_if_err(fd, "open (logfile)");
  }

  size_t offset = read_u64(fd);
  size_t len = read_u64(fd);
  char* data = cs644_readfile(fd);

  int r = close(fd);
  cs644_bail_if_err(r, "close (logfile)");

  return (struct logged_write){ .offset = offset, .len = len, .data = data };
}

// caller should `free` the result
char* make_dirpath(const char* filepath) {
  char* slash_pos = strrchr(filepath, '/');
  if (slash_pos == NULL) {
    return strdup(".");
  } else {
    char* r = strdup(filepath);
    size_t i = slash_pos - filepath;
    r[i] = '\0';
    return r;
  }
}

void sync_parent_dir(const char* filepath) {
  char* dirpath = make_dirpath(filepath);
  int fd = open(dirpath, O_RDONLY | O_DIRECTORY);
  cs644_bail_if_err(fd, "open");

  int r = fsync(fd);
  cs644_bail_if_err(r, "fsync");

  free(dirpath);
}

void log_write(const char* logpath, const char* s, size_t offset) {
  int fd = open(logpath, O_WRONLY | O_TRUNC | O_CREAT | O_EXCL, 0600);
  cs644_bail_if_err(fd, "open (logfile)");

  // simple format: 8 bytes for offset, 8 bytes for len, then rest of string
  write_u64(fd, offset);
  size_t n = strlen(s);
  write_u64(fd, n);
  ssize_t nwritten = write(fd, s, n);
  cs644_bail_if_err(nwritten, "write (logfile)");

  int r = fsync(fd);
  cs644_bail_if_err(r, "fsync (logfile)");

  r = close(fd);
  cs644_bail_if_err(r, "close (logfile)");

  sync_parent_dir(logpath);
}

void apply_logged_write(int fd, struct logged_write logged_write) {
  off_t new_offset = lseek(fd, logged_write.offset, SEEK_SET);
  cs644_bail_if_err(new_offset, "open");

  ssize_t nwritten = write(fd, logged_write.data, logged_write.len);
  cs644_bail_if_err(nwritten, "write");

  int r = fsync(fd);
  cs644_bail_if_err(r, "fsync");
}

void delete_log(const char* logpath) {
  int r = unlink(logpath);
  cs644_bail_if_err(r, "unlink");
  sync_parent_dir(logpath);
}

// write to file, writing to the log first
void save(const char* filepath, const char* s, size_t offset) {
  char* logpath = make_logpath(filepath);
  log_write(logpath, s, offset);
  
  int fd = open(filepath, O_WRONLY | O_CREAT, 0644);
  cs644_bail_if_err(fd, "open");

  apply_logged_write(fd, (struct logged_write){ .offset = offset, .len = strlen(s), .data = s });

  int r = close(fd);
  cs644_bail_if_err(r, "close");

  delete_log(logpath);
  free(logpath);
}

// simulate crashing in the middle of a write
void corrupt(const char* filepath) {
  int fd = open(filepath, O_WRONLY | O_TRUNC | O_CREAT, 0644);
  cs644_bail_if_err(fd, "open");

  // file is partially written out
  // initial contents: "aaaaa"
  // write "bbb" at offset=1 should yield "abbba"
  // but computer crashed before write was finished
  // logfile has intended write
  char* logpath = make_logpath(filepath);
  log_write(logpath, "bbb", 1);
  free(logpath);

  char* s = "abaaa";
  size_t n = strlen(s);
  ssize_t nwritten = write(fd, s, n);
  cs644_bail_if_err(nwritten, "write");

  int r = close(fd);
  cs644_bail_if_err(r, "close");
}

void print_logged_write(struct logged_write logged_write) {
  printf("offset: %ld\n", logged_write.offset);
  printf("len:    %ld\n", logged_write.len);
  printf("data:   \"%s\"\n", logged_write.data);
}

void recover(const char* filepath) {
  char* logpath = make_logpath(filepath);
  struct logged_write logged_write = read_log(logpath);

  if (logged_write.data == NULL) {
    puts("nothing to recover");
    goto cleanup;
  }

  print_logged_write(logged_write);

  int fd = open(filepath, O_WRONLY | O_CREAT, 0644);
  cs644_bail_if_err(fd, "open");

  apply_logged_write(fd, logged_write);

  int r = close(fd);
  cs644_bail_if_err(r, "close");

  delete_log(logpath);

cleanup:
  free(logpath);
}

int main(int argc, char* argv[]) {
  if (argc < 2) {
    usage(argv[0]);
  }

  if (strcmp(argv[1], "save") == 0) {
    if (!cs644_check_n_args(4, argc, argv)) { usage(argv[0]); }

    long long offset = cs644_str_to_int_or_bail(argv[4]);
    save(argv[2], argv[3], offset);
  } else if (strcmp(argv[1], "corrupt") == 0) {
    if (!cs644_check_n_args(2, argc, argv)) { usage(argv[0]); }

    corrupt(argv[2]);
  } else if (strcmp(argv[1], "recover") == 0) {
    if (!cs644_check_n_args(2, argc, argv)) { usage(argv[0]); }

    recover(argv[2]);
  } else if (strcmp(argv[1], "read-log") == 0) {
    if (!cs644_check_n_args(2, argc, argv)) { usage(argv[0]); }

    struct logged_write logged_write = read_log(argv[2]);
    if (logged_write.data == NULL) {
      puts("no log found");
    } else {
      print_logged_write(logged_write);
    }
  } else {
    usage(argv[0]);
  }
}
