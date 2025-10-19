/**
 * https://iafisher.com/cs644/spring2025/week7
 */
#include <errno.h>
#include <pthread.h>
#include <stdio.h>
#include "cs644.h"

pthread_mutex_t mut = PTHREAD_MUTEX_INITIALIZER;

void* mythread(void* p) {
  int r = pthread_mutex_lock(&mut);
  if (r != 0) { cs644_bail("pthread_mutex_lock failed"); }
  pthread_exit(NULL);
}

int main() {
  pthread_t thrd;
  int r = pthread_create(&thrd, NULL, mythread, NULL);
  if (r != 0) { cs644_bail("pthread_create failed"); }

  r = pthread_join(thrd, NULL);
  if (r != 0) { cs644_bail("pthread_join failed"); }

  r = pthread_mutex_trylock(&mut);
  if (r == EBUSY) {
    puts("Lock WAS NOT released by thread after pthread_exit.");
  } else if (r == 0) {
    puts("Lock WAS released by thread after pthread_exit.");
  } else {
    cs644_bail("pthread_mutex_trylock failed");
  }

  return 0;
}
