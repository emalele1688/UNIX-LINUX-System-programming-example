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
  
  if((ret = pthread_mutex_lock(&st->lock)) != 0)
  {
    printf("%s\n", strerror(ret));
    pthread_exit((void*)-1);
  }
  printf("Thread 1 entra in esecuzione\n");
  
  if(st->elem == NULL)
    st->elem = malloc(sizeof(type));
  //Sblocca il primo thread in wait. Se la coda dei thread in wait è vuota non fare nulla
  pthread_cond_signal(&st->cond);
  
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
  
  //Se elem è nullo attendo che venga allocato - Ogni wait invocata sarà sbloccata da una sola cond
  if(st->elem == NULL)
  {
    printf("Thread 2 in blocco\n");
    pthread_cond_wait(&st->cond, &st->lock);
    printf("Thread 2 esce dal blocco\n");
  }
  *st->elem = 20;
  
  pthread_mutex_unlock(&st->lock);
  
  pthread_exit(NULL);  
}


int main()
{
  pthread_t id[2];
  
  struct foo* arg = malloc(sizeof(struct foo));
  memset(arg, 0, sizeof(struct foo));
  
  pthread_mutex_init(&arg->lock, NULL);
  pthread_cond_init(&arg->cond, NULL);
  
  pthread_create(&id[0], NULL, thread_2, (void*)arg);
  pthread_create(&id[1], NULL, thread_1, (void*)arg);
  //Questo non servirà a nulla
  pthread_create(&id[1], NULL, thread_1, (void*)arg);
  sleep(1);
  printf("%d\n", *arg->elem);
  free(arg->elem);
  arg->elem = NULL;
      
  return 0;
}