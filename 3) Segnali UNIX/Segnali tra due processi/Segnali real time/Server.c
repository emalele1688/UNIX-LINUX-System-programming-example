#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>


int ds_pipe;

void close_server(void)
{
  printf("Chiusura server\n");
  close(ds_pipe);
  unlink("pipe");
}

void alrm_handler(int sig)
{
  printf("Timeout del server\n");
  close_server();
  exit(0);
}

void client_handler(int sig, siginfo_t* sinfo, void* context)
{
  printf("Signal messagge inviato dal client: %d\n", sinfo->si_value);
}


int main(int argn, char* args[])
{  
  if( mkfifo("pipe", O_CREAT|O_EXCL|0666) == -1)
  {
    printf("%s\n", strerror(errno));
    return -1;
  }
  
  ds_pipe = open("pipe", O_RDWR);
  if(ds_pipe == -1)
  {
    printf("%s\n", strerror(errno));
    return -1;
  }
  
  // 1) Invio il pid al client
  pid_t srvid = getpid();
  if(write(ds_pipe, &srvid, sizeof(srvid)) == -1)
  {
    printf("%s\n", strerror(errno));
    close_server();
    return -1;
  }
  
  // 2) Installo un gestore per il segnale SIGUSR1 inviato dal processo client
  struct sigaction sigusr;
  memset(&sigusr, 0, sizeof(sigusr));
  sigemptyset(&sigusr.sa_mask);
  sigusr.sa_flags = SA_SIGINFO;
  sigusr.sa_sigaction = client_handler;
  sigaction(SIGUSR1, &sigusr, NULL);
  
  /* 3) Il segnale SIGALRM deve poter sbloccare il processo se la pausa si prolunga troppo - lo blocco per poi sbloccarlo fuori la pausa
   * Il segnale SIGUSR1 deve essere bloccato fin quando il timeout non viene azzerato, per evitare il caso in cui arriva un SIGUSR1 ma prima della sua gestione si verifica un SIGALRM
   */
  //Installo prima un gestore per SIGALRM
  struct sigaction alrmsig;
  memset(&alrmsig, 0, sizeof(alrmsig));
  sigemptyset(&alrmsig.sa_mask);
  alrmsig.sa_handler = alrm_handler;
  sigaction(SIGALRM, &alrmsig, NULL);
  //Dopo blocco l'arrivo dei due segnali in modo da gestirli al momento giusto una volta riportato il processo in esecuzione
  sigset_t stop_mask, old_mask;
  sigemptyset(&stop_mask);
  sigaddset(&stop_mask, SIGALRM);
  sigaddset(&stop_mask, SIGUSR1);
  sigprocmask(SIG_BLOCK, &stop_mask, &old_mask);
  
  // 4) Predispongo il timer
  printf("Attendo messaggi dal client\n");
  alarm(30);
  
  // 5) Porto il processo in pausa fin quando non sarà ricevuto il segnale SIGALRM o SIGUSR1
  sigset_t sleep_mask;
  sigemptyset(&sleep_mask);
  sigaddset(&sleep_mask, SIGINT); //Durante il periodo in cui il processo è in pausa blocco l'arrivo del segnale SIGINT
  sigsuspend(&sleep_mask); //Il processo viene bloccato ed i segnali presenti in sleep_mask non permetteranno di riprendere l'esecuzione del processo
  
  // 6) Resetto il timeout e sblocco il segnale SIGALRM
  alarm(0);
  sigprocmask(SIG_SETMASK, &old_mask, NULL); //A questo punto, se il processo è stato rimesso in esecuzione per l'arrivo del SIGALRM allora termina, altrimenti gestisce il segnale del client 
  
  close_server();

  return 0;
}