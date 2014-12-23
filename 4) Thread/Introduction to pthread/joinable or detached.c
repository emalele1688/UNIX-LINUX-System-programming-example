#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <errno.h>


void* joinable_thread(void* arg)
{
  //Il valore di ritorno (come per gli argomenti passati alla funzione thread) devono essere ancora validi quando lo stack di riferimento termina
  int* ret = malloc(4);
  *ret = 40;
  pthread_t tid = pthread_self();
  printf("sono il thread %u (joined)\n", tid);
  //sleep addormenta solo il thread chiamante
  sleep(5);
  printf("%u: Leggi il mio codice di ritorno dopo.. Bye!!\n", tid);
  pthread_exit(ret);
}

void* detachable_thread(void* arg)
{
  pthread_t tid = pthread_self();
  printf("Sono il thread %u (detached) e ti dico che arg contiene %d\n", tid, *((int*)arg));
  sleep(1);
  printf("%u: Sto per terminare, poichè sono detached non puoi fare join con me, non leggi il mio codice di ritorno\n", tid);
  pthread_exit(0);
}

void* autodetach_thread(void* arg)
{
  //Auto detach
  pthread_detach(pthread_self());
  pthread_t tid = pthread_self();
  printf("Sono il thread %u (autodetachable)\n", tid);
  sleep(2);
  printf("%u: Sto per terminare, poichè sono detached non puoi fare join con me, non leggi il mio codice di ritorno\n", tid);
  pthread_exit(0);
}

int main(int argn, char** args)
{
  //Lo stato di ritorno è un puntatore ad un'area di memoria allocata da me
  void *status;
  int ret;
  
  pthread_t id[3];
  pthread_attr_t attr;
  
  //pthread_attr_setdetachstate permette di impostare un thread in modo da essere o joinable o detachable
  pthread_attr_init(&attr);
  pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
  
  if( (ret = pthread_create(&id[0], &attr, joinable_thread, NULL)) != 0)
    printf("joinable_thread non creato: %s\n", strerror(ret));
  
  //Il metodo più sicuro per passare attributi ad un thread è quello di NON allocarli nello stack delle funzioni (soprattutto se non si tratta del main)
  int* thread_arg = malloc(4);
  *thread_arg = 3;
  pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
  if( (ret = pthread_create(&id[1], &attr, detachable_thread, thread_arg)) != 0)
    printf("detachable_thread non creato: %s\n", strerror(ret));
  
  if( (ret = pthread_create(&id[2], NULL, autodetach_thread, NULL)) != 0)
    printf("autodetach_thread non creato: %s\n", strerror(ret));
  
  pthread_attr_destroy(&attr);
  
  //Posso mettermi in attesa solo del joinable_thread
  pthread_join(id[0], &status);
  printf("joinable ha rilasciato un codice di ritorno: %d\n", *((int*)status));
  
  return 0;
}
