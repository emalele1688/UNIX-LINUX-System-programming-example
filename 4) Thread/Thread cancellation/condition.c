#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>


typedef int type;

struct foo
{
  pthread_mutex_t lock;
  pthread_cond_t cond;
  type* elem;
};


void* thread_1(void* arg)
{
  int ret;
  struct foo *st = (struct foo*)arg;
  
  
  sleep(2);
  pthread_cond_signal(&st->cond);
  pthread_cond_wait(&st->cond, &st->lock);
  
  pthread_exit(NULL);  
}

void* thread_2(void* arg)
{
  
  
  pthread_exit(NULL);  
}


int main()
{
  pthread_t id[2];
  pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, NULL);
  struct foo* arg = malloc(sizeof(struct foo));
  memset(arg, 0, sizeof(struct foo));
  
  pthread_mutex_init(&arg->lock, NULL);
  pthread_cond_init(&arg->cond, NULL);
  
  pthread_create(&id[0], NULL, thread_2, (void*)arg);
  pthread_create(&id[1], NULL, thread_1, (void*)arg);
  //Questo non servirà a nulla
  //pthread_create(&id[1], NULL, thread_1, (void*)arg);
  
  pthread_join(id[0], NULL);
  printf("Thread 2 bye\n");
  pthread_join(id[1], NULL);
  
  return 0;
}
