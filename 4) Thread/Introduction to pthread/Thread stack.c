#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>
#include <limits.h> // Definisce PTHREAD_STACK_MIN

pthread_attr_t attr;

void* stack_thread(void* args)
{
  int ret;
  size_t stacksize;
  
  if( (ret = pthread_attr_getstacksize(&attr, &stacksize)) != 0)
    printf("Impossibile prelevare la grandezza dello stack %s\n", strerror(ret));
  printf("%li\n", stacksize);
  
  pthread_exit(NULL);
}

int main(int argn, char* args[])
{
  int ret;
  pthread_t id;
   
  printf("dimensione minima di stack per ogni thread: %u\n", PTHREAD_STACK_MIN);
  
  pthread_attr_init(&attr);
  //Imposto la dimensione dello stack del thread
  if( (ret = pthread_attr_setstacksize(&attr, PTHREAD_STACK_MIN + 512)) != 0)
    printf("Impossibile assegnare una dimensione allo stack del prossimo thread: %s\n", strerror(ret));
  if( (ret = pthread_create(&id, &attr, stack_thread, NULL)) != 0)
    printf("Impossibile creare il thread %s\n", strerror(ret));
  
  pthread_join(id, NULL);
  
  return 0;
}