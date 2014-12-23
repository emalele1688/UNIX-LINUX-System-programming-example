#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <setjmp.h>
#include <string.h>

sigjmp_buf env;


void allarm(int sig)
{
  raise(SIGINT);
  printf("Allarmeeee!\n");
  siglongjmp(env, 1);
}

void interrupt(int sig)
{
  printf("interrupt catturato\n");
}


int main(int argn, char* args[])
{
  struct sigaction alrm_sigact;
  struct sigaction int_sigact;
  
  memset(&alrm_sigact, 0, sizeof(alrm_sigact));
  sigemptyset(&alrm_sigact.sa_mask);
  sigaddset(&alrm_sigact.sa_mask, SIGINT);
  alrm_sigact.sa_handler = allarm;
  
  memset(&int_sigact, 0, sizeof(int_sigact));
  sigemptyset(&int_sigact.sa_mask);
  int_sigact.sa_handler = interrupt;
  
  sigaction(SIGALRM, &alrm_sigact, NULL);
  sigaction(SIGINT, &int_sigact, NULL);
  
  /* sigsetjmp imposta il punto dove la siglongjmp deve tornare
   * rispetto alla normale setjmp, sigsetjmp permette di sbloccare i segnali pendenti che vengono serviti in ordine seguendo il signal set impostato dalla sigaction
   * il secondo parametro abbilita appunto questa capacità (1 o 0). Se 0 i segnali bloccati nel gestore (SIGALRM nll'esempio) non verrano mai serviti
   */
  if(sigsetjmp(env, 1) == 1)
  {
    printf("Ritorno dal gestore del segnale\nBye Bye!\n");
    exit(0);
  }
  
  //Se non sto tornando dal gestore del segnale
  alarm(4);
  pause();  
  
  return 0;
}