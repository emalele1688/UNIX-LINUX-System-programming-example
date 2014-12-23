#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>


struct foo
{
  pthread_rwlock_t lock;
  int val;
};

void* thread_1(void* arg)
{
  int ret;
  struct foo *st = (struct foo*)arg;
  
  //BLOCCO IN SCRITTURA
  if((ret = pthread_rwlock_wrlock(&st->lock)) != 0)
  {
    printf("%s\n", strerror(ret));
    pthread_exit((void*)-1);
  }
  
  printf("Scrittura del valore nella variabile\n"); 
  st->val = 10;
  sleep(2);
  
  pthread_rwlock_unlock(&st->lock); //=> Ora thread_2 può acquisire il lock
  
  pthread_exit(NULL);  
}

void* thread_2(void* arg)
{
  int ret;
  struct foo *st = (struct foo*)arg;
  
  //BLOCCO IN LETTURA
  if((ret = pthread_rwlock_rdlock(&st->lock)) != 0)
  {
    printf("%s\n", strerror(ret));
    pthread_exit((void*)-1);
  }
  
  printf("Il valore della variabile è %d\n", st->val); 
  
  pthread_rwlock_unlock(&st->lock);
  
  pthread_exit(NULL);  
}

int main()
{
  pthread_t id[2];
  
  struct foo* arg = malloc(sizeof(struct foo));
  //read-write lock => (La lettura attende la scrittura)
  pthread_rwlock_init(&arg->lock, NULL);
  
  pthread_create(&id[0], NULL, thread_1, (void*)arg);
  pthread_create(&id[1], NULL, thread_2, (void*)arg);
  
  pthread_join(id[0], NULL);
  pthread_join(id[1], NULL);
  
  return 0;
} 
