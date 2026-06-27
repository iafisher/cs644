#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

void* my_thread(void* arg) {
  printf("I am my own thread. (%ld)\n", pthread_self());
  sleep(5);
  puts("Thread returning...");
  return NULL;
}

int main() {
  pthread_t myself = pthread_self();
  printf("main thread: %ld\n", myself);

  pthread_t thrd;
  int r = pthread_create(
      &thrd, NULL, my_thread, NULL
  );

  if (r < 0) {
    perror("pthread_create");
    return 1;
  }

  puts("main thread executing");

  r = pthread_join(thrd, NULL);
  if (r < 0) {
    perror("pthread_join");
    return 1;
  }

  return 0;
}
