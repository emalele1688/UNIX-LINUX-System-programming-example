#include <unistd.h>
#include <stdio.h>
#include <signal.h>
#include <pthread.h>
#include <string.h>
#include <semaphore.h>

sem_t sem;

pthread_t dadThread;
pthread_t childThread;

int sendSignal = 0;

void interruptSig(int sig)
{
  printf("Gestore eseguito da %d\n", pthread_self());
  
  if(pthread_self() == dadThread)
  {
    printf("Sono il thread padre, faccio terminare mio figlio\n");
    sem_post(&sem);
  }
  
  if(pthread_self() == childThread)
    printf("Sono il thread figlio e termino\n");
}

void *child(void* arg)
{
  printf("Salve sono il thread figlio %u, entro in pausa finchè mio padre fa una scelta\n", childThread);

  sem_wait(&sem);
  printf("Child esce dall'attesa\n");
  if(sendSignal == 1)
    //invio il segnale SIGINT al thread figlio
    raise(SIGINT);
  
  printf("Child thread in chiusura\n");  
  pthread_exit(0);
}

int main(int argn, char* args[])
{
  int ret;
  struct sigaction sigint;
  
  sem_init(&sem, 0, 0);
  dadThread = pthread_self();
  memset(&sigint, 0, sizeof(sigint));
  
  sigaddset(&sigint.sa_mask, SIGINT);
  sigint.sa_handler = interruptSig;
  sigaction(SIGINT, &sigint, NULL);
  
  printf("Sono il padre con id %u\n", dadThread);
  
  if((ret = pthread_create(&childThread, NULL, child, NULL)) == -1)
  {
    printf("%s\n", strerror(ret));
    return -1;
  }
  
  printf("Premi:\n\t1 per lanciare il segnale dal figlio\n\t2 per lanciare il segnale dal padre\n");
  scanf("%d", &sendSignal);
  if(sendSignal == 1)
    sem_post(&sem);
  else
    //invio il segnale SIGINT al thread padre
    raise(SIGINT);
  
  pthread_join(childThread, NULL);
  
  printf("Terminazione padre\n");
  
  return 0;
}

/* REMEMBER: raise è come pthread_kill(pthread_self(), SIG). Il segnale sarà consegnato al singolo thread.
 * Dunque solo il thread che riceve il segnale eseguira il gestore del segnale. L'uscita dalla syscall è per il singolo thread
 * L'installazione dei gestori (sigaction) è valida su tutti i thread
 */