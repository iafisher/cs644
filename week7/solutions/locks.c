#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include "cs644.h"

static pthread_mutex_t mut = PTHREAD_MUTEX_INITIALIZER;

void* mythread(void* p) {
  puts("thread 1 running");
  int r = pthread_mutex_lock(&mut);
  if (r != 0) { cs644_bail("pthread_mutex_lock failed"); }

  puts("thread 1 acquired lock");
  sleep(3);
  pthread_mutex_unlock(&mut);
  puts("thread 1 exiting");
  return NULL;
}

int main() {
  pthread_t thrd;
  int r = pthread_create(&thrd, NULL, mythread, NULL);
  if (r != 0) { cs644_bail("pthread_create failed"); }

  r = pthread_mutex_lock(&mut);
  if (r != 0) { cs644_bail("pthread_mutex_lock failed"); }

  puts("main thread acquired lock");
  sleep(3);
  pthread_mutex_unlock(&mut);

  r = pthread_join(thrd, NULL);
  if (r != 0) { cs644_bail("pthread_join failed"); }

  return 0;
}
