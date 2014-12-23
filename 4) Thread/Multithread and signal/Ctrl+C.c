#include <unistd.h>
#include <stdio.h>
#include <signal.h>
#include <pthread.h>
#include <string.h>
#include <semaphore.h>

sem_t sem;

pthread_t dadThread;
pthread_t childThread;

void interruptSig(int sig)
{
  printf("Gestore eseguito da %u\n", pthread_self());
  
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
  
  sigset_t sig, oldsig;
  sigemptyset(&sig);
  sigaddset(&sig, SIGINT);
  //Blocco il segnale SIGINT => Questo comportamento è ereditato dagli altri thread che manterranno il segnale SIGINT bloccato
  pthread_sigmask(SIG_BLOCK, &sig, &oldsig);
  
  //Installo un gestore per SIGINT
  sigint.sa_mask = sig;
  sigint.sa_handler = interruptSig;
  sigaction(SIGINT, &sigint, NULL);
  
  printf("Sono il padre con id %u\nCreo mio figlio ed entro in pausa\nRicordati di sbloccarmi con Ctrl+C\n", dadThread);
  
  //Avvio il thread figlio
  if((ret = pthread_create(&childThread, NULL, child, NULL)) == -1)
  {
    printf("%s\n", strerror(ret));
    return -1;
  }
  
  //Sblocco la ricezione di SIGINT su questo thread => I thread figli (che mantengono SIGINT bloccato) non intercetteranno mai SIGINT
  pthread_sigmask(SIG_SETMASK, &oldsig, NULL);
  
  pause();
  printf("Dad esce dall'attesa\n");
  
  pthread_join(childThread, NULL);
  
  printf("Terminazione padre\n");
  
  return 0;
} 

/* WARNING: Se mantengo bloccato sul main thread il segnale SIGINT ma lo sblocco nel child thread il segnale sarà intercettato dal child thread (quindi il main thread resterà per sempre in pausa)
 * Se non blocco per nessun thread il segnale SIGINT il kernel provvederrà ad inviare tale segnale ad un solo thread del processo arbitrariamente (molto spesso il main thread)
