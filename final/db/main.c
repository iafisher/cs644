#include <fcntl.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "cs644.h"

// TODO: move `dbfile_*` functions into own file
// TODO: `read` command
// TODO: `clear` command
// TODO: better documentation

const char* DB_FILE = "/home/ian/final.db";

void usage(void);

#define MAX_KEY_SIZE 100
#define MAX_VAL_SIZE 4000
#define BLOCK_SIZE (MAX_KEY_SIZE + MAX_VAL_SIZE + 2)

int dbfile_open() {
  int fd = open(DB_FILE, O_CREAT | O_RDWR | O_APPEND, 0644);
  handle_err(fd, "open (dbfile_open)");
  return fd;
}

struct db_entry {
  char* block;
};

char* db_entry_key(struct db_entry entry) {
  return entry.block;
}

char* db_entry_val(struct db_entry entry) {
  if (entry.block == NULL) {
    return NULL;
  }
  return entry.block + MAX_KEY_SIZE + 1;
}

struct db_entry db_entry_empty() {
  return (struct db_entry){ .block = NULL };
}

bool db_entry_is_empty(struct db_entry entry) {
  return entry.block == NULL;
}

void db_entry_free(struct db_entry entry) {
  free(entry.block);
}

struct db_entry dbfile_read(int handle) {
  // disk format: flat list of blocks
  char* block = malloc_s(BLOCK_SIZE);
  ssize_t bytes_read = read(handle, block, BLOCK_SIZE);
  handle_err(bytes_read, "read (dbfile_read)");
  if (bytes_read == 0) {
    return db_entry_empty();
  } else if (bytes_read != BLOCK_SIZE) {
    // TODO: handle this more gracefully
    bail("failed to read block");
  }

  if (block[MAX_KEY_SIZE] != '\0') {
    bail("db file corrupted (key not null-terminated)");
  }

  if (block[BLOCK_SIZE - 1] != '\0') {
    bail("db file corrupted (block not null-terminated)");
  }

  return (struct db_entry){ .block = block };
}

void dbfile_append(int handle, const char* key, const char* val) {
  // TODO: check that key and val do not exceed max length
  // TODO: extract this out into `db_entry_new` function
  char padded_key[MAX_KEY_SIZE + 1];
  char padded_val[MAX_VAL_SIZE + 1];

  strncpy(padded_key, key, MAX_KEY_SIZE);
  padded_key[MAX_KEY_SIZE] = '\0';
  strncpy(padded_val, val, MAX_VAL_SIZE);
  padded_val[MAX_VAL_SIZE] = '\0';

  ssize_t r = write(handle, padded_key, MAX_KEY_SIZE + 1);
  handle_err(r, "write (dbfile_append)");
  r = write(handle, padded_val, MAX_VAL_SIZE + 1);
  handle_err(r, "write (dbfile_append)");
}

void dbfile_close(int handle) {
  int r = fsync(handle);
  handle_err(r, "fsync (dbfile_close)");
  r = close(handle);
  handle_err(r, "close (dbfile_close)");
}

struct db_entry db_get(const char* key) {
  int handle = dbfile_open();

  struct db_entry r = db_entry_empty();
  while (1) {
    struct db_entry entry = dbfile_read(handle);
    if (db_entry_is_empty(entry)) {
      break;
    } else if (strcmp(db_entry_key(entry), key) == 0) {
      r = entry;
    }
  }
  return r;
}

void db_set(const char* key, const char* val) {
  int handle = dbfile_open();

  dbfile_append(handle, key, val);

  dbfile_close(handle);
}

int main(int argc, char* argv[]) {
  if (argc < 2) {
    usage();
  }

  const char* cmd = argv[1];
  if (strcmp(cmd, "get") == 0) {
    if (argc != 3) {
      usage();
    }

    struct db_entry entry = db_get(argv[2]);
    if (db_entry_is_empty(entry)) {
      puts("<not found>");
      return 1;
    } else {
      printf("%s\n", db_entry_val(entry));
      db_entry_free(entry);
    }
  } else if (strcmp(cmd, "set") == 0) {
    if (argc != 4) {
      usage();
    }

    db_set(argv[2], argv[3]);
  } else {
    usage();
  }

  return 0;
}

void usage() {
  fputs("usage: ./db [cmd]\n", stderr);
  exit(1);
}
