#include <stdio.h>
#include <sys/stat.h>
#include "cs644.h"

void print_perms(int p) {
  char r = p & 4 ? 'r' : '-';
  char w = p & 2 ? 'w' : '-';
  char x = p & 1 ? 'x' : '-';
  printf("%c%c%c", r, w, x);
}

int main(int argc, char* argv[]) {
  if (argc != 2 || argv[1][0] == '-') {
    return 1;
  }

  const char* filename = argv[1];
  struct stat statbuf;
  int r = stat(filename, &statbuf);
  handle_err(r, "stat");

  print_perms(statbuf.st_mode >> 6);
  print_perms(statbuf.st_mode >> 3);
  print_perms(statbuf.st_mode);
  puts("");

  return 0;
}
