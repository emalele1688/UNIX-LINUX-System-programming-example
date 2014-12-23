#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>


struct foo
{
  pthread_mutex_t lock;
  int val;
};


void* thread_1(void* arg)
{
  int ret;
  struct foo *st = (struct foo*)arg;
  
  if((ret = pthread_mutex_lock(&st->lock)) != 0)
  {
    printf("%s\n", strerror(ret));
    pthread_exit((void*)-1);
  }
  printf("Thread 1 entra in esecuzione\n");
  
  st->val = 10;
  sleep(2);
  printf("Thread 1: %d\n", st->val);
  
  pthread_mutex_unlock(&st->lock);
  
  pthread_exit(NULL);  
}

void* thread_2(void* arg)
{
  int ret;
  struct foo *st = (struct foo*)arg;
  
  if((ret = pthread_mutex_lock(&st->lock)) != 0)
  {
    printf("%s\n", strerror(ret));
    pthread_exit((void*)-1);
  }
  printf("Thread 2 entra in esecuzione\n");
  
  st->val = 100;
  sleep(4);
  printf("Thread 2: %d\n", st->val);
  
  pthread_mutex_unlock(&st->lock);
  
  pthread_exit(NULL);  
}

void* thread_3(void* arg)
{
  int ret;
  struct foo *st = (struct foo*)arg;
  
  if((ret = pthread_mutex_lock(&st->lock)) != 0)
  {
    printf("%s\n", strerror(ret));
    pthread_exit((void*)-1);
  }
  printf("Thread 3 entra in esecuzione\n");
  
  st->val = 18;
  printf("Thread 3: %d\n", st->val);
  
  pthread_mutex_unlock(&st->lock);
  
  pthread_exit(NULL);  
}


int main()
{
  pthread_t id[3];
  
  struct foo* arg = malloc(sizeof(struct foo));
  pthread_mutex_init(&arg->lock, NULL);
  
  pthread_create(&id[0], NULL, thread_1, (void*)arg);
  pthread_create(&id[1], NULL, thread_2, (void*)arg);
  pthread_create(&id[2], NULL, thread_3, (void*)arg);
  
  pthread_join(id[0], NULL);
  pthread_join(id[1], NULL);
  pthread_join(id[2], NULL);
  
  printf("Nota: non è garantito l'ordine di esecuzione dei thread\n");
  
  return 0;
}