#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <string.h>
#include <errno.h>


sem_t sem;


void* thread(void* arg)
{
  pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, NULL);
  
  printf("Sono il thread %u\nEntro in attesa di uno sblocco\n", pthread_self());
  
  sem_wait(&sem);
  
  printf("%u: Byee!!\n", pthread_self());
  
  pthread_exit(0);
}

int main(int argn, char** args)
{
  pthread_t id;
  int ret;
  
  if(sem_init(&sem, 0, 0) == -1)
    printf("%s\n", strerror(errno));
  
  if( (ret = pthread_create(&id, NULL, thread, NULL)) != 0)
  {
    printf("thread non creato: %s\n", strerror(ret));
    exit(1);
  }

  printf("Dad thread: tra 5 secondi cancello il thread avviato che è in attesa perenne\n");
  sleep(5);
  pthread_cancel(id);
  //Dopo averlo cancellato il thread continua a rimanere nello stato di zombie fin quando non si invoca pthread_join
  pthread_join(id, NULL);
  
  if(sem_destroy(&sem) == -1)
    printf("%s\n", strerror(errno));
  
  printf("Uscita\n");
  
  return 0;
}
 
