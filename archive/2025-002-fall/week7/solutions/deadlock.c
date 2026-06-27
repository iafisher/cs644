/**
 * https://iafisher.com/cs644/spring2025/week7
 */
#include <pthread.h>
#include <stdio.h>
#include "cs644.h"

pthread_mutex_t mut1 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mut2 = PTHREAD_MUTEX_INITIALIZER;

void* mythread(void* p) {
  for (int i = 0; i < 100; i++) {
    printf("thread 1: i=%d\n", i);
    int r = pthread_mutex_lock(&mut2);
    if (r != 0) { cs644_bail("pthread_mutex_lock failed"); }

    cs644_sleep_millis(50);

    r = pthread_mutex_lock(&mut1);
    if (r != 0) { cs644_bail("pthread_mutex_lock failed"); }

    cs644_sleep_millis(10);

    r = pthread_mutex_unlock(&mut1);
    if (r != 0) { cs644_bail("pthread_mutex_unlock failed"); }
    r = pthread_mutex_unlock(&mut2);
    if (r != 0) { cs644_bail("pthread_mutex_unlock failed"); }
  }

  return NULL;
}

int main() {
  pthread_t thrd;
  int r = pthread_create(&thrd, NULL, mythread, NULL);
  if (r != 0) { cs644_bail("pthread_create failed"); }

  for (int i = 0; i < 100; i++) {
    printf("main thread: i=%d\n", i);
    r = pthread_mutex_lock(&mut1);
    if (r != 0) { cs644_bail("pthread_mutex_lock failed"); }
    r = pthread_mutex_lock(&mut2);
    if (r != 0) { cs644_bail("pthread_mutex_lock failed"); }

    cs644_sleep_millis(10);

    r = pthread_mutex_unlock(&mut2);
    if (r != 0) { cs644_bail("pthread_mutex_unlock failed"); }
    r = pthread_mutex_unlock(&mut1);
    if (r != 0) { cs644_bail("pthread_mutex_unlock failed"); }
  }

  r = pthread_join(thrd, NULL);
  if (r != 0) { cs644_bail("pthread_join failed"); }

  return 0;
}
