#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>

pthread_barrier_t barrier;

void *thread1(void *arg)
{
  time_t now;

  time(&now);
  printf("thread1 starting at %s", ctime(&now));

  // do the computation
  // let's just do a sleep here...
  sleep(5);
  pthread_barrier_wait(&barrier);
  printf("thread1 came out from the barrier\n");
  // after this point, all three threads have completed.
  time(&now);
  printf("barrier in thread1() done at %s", ctime(&now));
  
  pthread_exit(NULL);
}

void *thread2(void *arg)
{
  time_t now;

  time(&now);
  printf("thread2 starting at %s", ctime(&now));

  // do the computation
  // let's just do a sleep here...
  sleep(10);
  pthread_barrier_wait(&barrier);
  printf("thread2 came out from the barrier\n");
  // after this point, all three threads have completed.
  time (&now);
  printf ("barrier in thread2() done at %s", ctime(&now));
  
  pthread_exit(NULL);
}

int main()
{
  pthread_t id[2];

  // create a barrier object with a count of 3
  pthread_barrier_init(&barrier, NULL, 2);

  // start up two threads, thread1 and thread2
  pthread_create(&id[0], NULL, thread1, NULL);
  pthread_create(&id[1], NULL, thread2, NULL);

  pthread_join(id[0], NULL);
  pthread_join(id[1], NULL);
  
  pthread_barrier_destroy(&barrier);

  return 0;
}
