#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <errno.h>


void* thread(void* arg)
{
  printf("Sono il thread %u\n", pthread_self());
  pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);
  printf("%u: Dormo 5 secondi, non puoi cancellarimi\n", pthread_self());
  sleep(5);
  pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
  printf("%u: Dormo 10 secondi, ma ora puoi cancellarimi\n", pthread_self());
  sleep(10);
  printf("%u: Sto per terminare\n", pthread_self());
  pthread_exit(0);
}

int main(int argn, char** args)
{
  pthread_t id;
  int ret;
  
  if( (ret = pthread_create(&id, NULL, thread, NULL)) != 0)
  {
    printf("thread non creato: %s\n", strerror(ret));
    exit(1);
  }

  printf("Elimino il thread appena avviato\n");
  pthread_cancel(id);
  printf("Attendo che il thread viene cancellato\n");
  //Dopo averlo cancellato il thread continua a rimanere nello stato di zombie fin quando non si invoca pthread_join
  pthread_join(id, NULL);
  printf("Uscita\n");
  
  return 0;
}
