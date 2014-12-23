#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>
#include <semaphore.h>


int val = 0;
sem_t sem;
sem_t sem2;
 pthread_t id[3];

void* thread_1(void* arg)
{
  //Parte per primo
  printf("Thread 1 entra in esecuzione\n");
  
  val = 10;
  printf("Thread 1: %d\n", val);
  
  sem_wait(&sem);
  
  pthread_exit(NULL);  
}

void* thread_2(void* arg)
{
  sleep(2);
  
  
  
  if(sem_wait(&sem) == -1)
    pthread_exit((void*)-1);
  printf("Thread 2 entra in esecuzione\n");
  
  val = 100;
  printf("Thread 2: %d\n", val);

  sem_post(&sem2);
  
  pthread_exit(NULL);  
}

void* thread_3(void* arg)
{
  sleep(5);
  sem_post(&sem);
  pthread_exit(NULL);  
}


int main()
{
 pthread_setcancelstate(PTHREAD_CANCEL_DEFERRED, NULL);
  
  sem_init(&sem, 0, 0);
  int ret = sem_init(&sem, 0, 0);
  
  sem_init(&sem2, 0, 0);  
  
  pthread_create(&id[0], NULL, thread_1, NULL);
  pthread_create(&id[1], NULL, thread_2, NULL);
  pthread_create(&id[2], NULL, thread_3, NULL);
  
  pthread_join(id[0], NULL);
  printf("Bye\n");
  pthread_join(id[1], NULL);
  pthread_join(id[2], NULL);
  
  printf("Nota: non è garantito l'ordine di esecuzione dei thread\n");
  
  return 0;
} 
