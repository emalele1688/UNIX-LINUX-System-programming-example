#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <errno.h>

void gestione_terminazione_figlio()
{
  printf("Parent: Processo figlio terminato\n");
}

void childProccess(int sec)
{
  printf("Child: I'm alive!\n");
  alarm(2);
  //Se il segnale arriva mentre il processo è in sleep, allora la chiamata si interrompe e il processo riprende l'esecuzione, eseguendo prima il gestore allarme, poi riparte da qui
  int ret = sleep(sec);

  printf("Child: Uscita del processo figlio numero %d con errore: %s\n", getpid(), strerror(errno));
  printf("Tempo che sarebbe rimasto da attendere per sleep %d\n", ret);
  
  exit(0);
}

void allarme()
{
  printf("Allarmeeeee!!\n");
}

int main(int arg, char* args[])
{ 
  /* Installo un gestore di segnale per SIGCHLD e SIGALRM
   * In tal modo, il segnale è intercettato e la sua funzione di default sarà sostituita con l'esecuzione del gestore.
   * Per cui all'arrivo di SIGALRM il processo non termina, esegue la routine allarme e poi riprende l'esecuzione regolarmente
   * All'arrivo di SIGCHLD il segnale non sarà ignorato, ma sarà eseguita la routine gestione_terminazione_figlio, per poi riprendere l'esecuzione
   */
  signal(SIGCHLD, gestione_terminazione_figlio);
  signal(SIGALRM, allarme);

  pid_t child1 = fork();
  if(child1 == 0)
    childProccess(10);
  
  pid_t child2 = fork();
  if(child2 == 0)
    childProccess(10);
  
  //Attesa di tutti i processi figli
  int ret;
  do
    //Attendo per un qualsiasi processo figlo, Non prendo lo stato (NULL), nessun opzione. Torna il pid del processo terminato
    ret = waitpid(-1, NULL, 0);
  while(ret > 0); //Finchè ho processi figli da attendere

  printf("Ciaooo!\n");
  
  return 0;
}
