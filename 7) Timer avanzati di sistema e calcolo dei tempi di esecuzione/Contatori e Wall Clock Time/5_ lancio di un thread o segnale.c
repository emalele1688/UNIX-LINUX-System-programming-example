#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <time.h>
#include <string.h>
#include <errno.h>

#define NANO_SECOND 1000000000

/* timer_settime è come clock_gettime, solo che avvia un orologgio creato con timer_create, che permette di dare maggiori funzionalità all'evento timer (invio di un segnale o avvio di un thread allo scadere del tempo */

void timeout(int sig)
{
  printf("timeout\n");
}

int main(int argn, char* args)
{
  struct sigaction sig;
  struct sigevent evt;
  struct itimerspec its;
  struct timespec it;
  timer_t timerid;
  sigset_t sigset;
  
  sigemptyset(&sig.sa_mask);
  sig.sa_flags = 0;
  sig.sa_handler = timeout;
  if(sigaction(SIGRTMIN, &sig, NULL) == -1)
    printf("Non è stato possibile assegnare un gestore al segnale SIGRTMIN: %s\n", strerror(errno));
  
  //Blocco il segnale SIGRTMIN
  sigemptyset(&sigset);
  sigaddset(&sigset, SIGRTMIN);
  //Blocca i segnali nel set: SIG_SETMASK il set è quello specificato in sigset
  sigprocmask(SIG_SETMASK, &sigset, NULL);
  
  //Tipo di notifica = Invia un segnale
  evt.sigev_notify = SIGEV_SIGNAL;
  //Segnale da inviare
  evt.sigev_signo = SIGRTMIN;
  evt.sigev_value.sival_ptr = &timerid;
  if(timer_create(CLOCK_REALTIME, &evt, &timerid) == -1)
    printf("Errore nel creare il timer %s\n", strerror(errno));
  
  its.it_value.tv_sec = 1;
  its.it_value.tv_nsec = 0;
  its.it_interval.tv_sec = its.it_value.tv_sec;
  its.it_interval.tv_nsec = its.it_value.tv_nsec;
  if(timer_settime(timerid, 0, &its, NULL) == -1)
    printf("Errore nell'impostare il timer %s\n", strerror(errno));
  
  //Sblocco il segnale precedentemente bloccato, che mi dovrà far uscire dalla pausa del processo. Se cosi non faccio il processo va in deadlock
  sigprocmask(SIG_UNBLOCK, &sigset, NULL);
  printf("Entro in attesa del timer\n");
  pause();
    
  return 0;
}

