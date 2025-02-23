#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char* argv[]) {
  if (argc != 2 || *argv[1] == '-') {
    fputs("error: redact takes a single argument\n", stderr);
    exit(1);
  }

  for (const char* s = argv[1]; *s; s++) {
    if (isdigit(*s)) {
      printf("X");
    } else {
      printf("%c", *s);
    }
  }
  printf("\n");

  return 0;
}
