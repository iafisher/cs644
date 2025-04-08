#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include "cs644.h"

void* mythread(void* p) {
  puts("thread 1 running");
  sleep(3);
  puts("thread 1 exiting");
  return NULL;
}

int main() {
  pthread_t thrd;
  int r = pthread_create(&thrd, NULL, mythread, NULL);
  if (r != 0) { cs644_bail("pthread_create failed"); }
  sleep(1);
  puts("main exiting");
  return 0;
}
