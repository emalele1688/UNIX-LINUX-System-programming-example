#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h> 
#include <sys/wait.h>
#include <signal.h>

void sigchld(int sig)
{
  printf("Il figlio è terminato\n");
}

int childExecute(void)
{
  int i = 0;
  while(i < 100000000)
    i++;
  
  return 1;
}

int main(int argn, char* args[])
{
  pid_t chpid;
  int childstatus;
  
  signal(SIGCHLD, &sigchld);
  
  printf("Pid del processo: %d\nCreo un processo figlio\n", getpid());
  chpid = fork();
  if(chpid == 0)
  {
    printf("Ciao, sono il processo figlio con id: %d\n", getpid());
    int ret = childExecute();
    printf("Processo figlio terminato\n");
    exit(ret);
  }
  else if(chpid == -1)
  {
    printf("%s\n", strerror(errno));
    exit(1);
  }
  
  printf("Il processo padre entra in attesa del processo figlio\n");
  /* Alla conclusione del primo processo figlio wait ritorna, e scrive in status il valore di ritorno del processo figlio tornato.
   * Se il processo figlio termina prima che il padre invochi wait, il processo figlio si mette in stato di zombie. Quando il padre invoca wait, wait trasferisce al padre il codice di ritorno del figlio e dealloca completamente il processo
   * Se il padre termina prima del figlio, il figlio viene ereditato da init (systemd, initd ecc...). In tal caso il processo non avrà più alcun terminale associato ma potrà continuare a inviare messaggi sullo standard output.
   * wait torna il pid del figlio che ha terminato l'esecuzione
   */
  //wait(&childstatus);
  /* Uso waitpid:
   * pid, stato di ritotno, option
   */
  waitpid(chpid, &childstatus, 0);
  
  //Eseguo vari controlli sullo stato di uscita del child
  if(WIFEXITED(childstatus))
    printf("Child uscito normalmente\n");
  if(WIFSIGNALED(childstatus))
    printf("Child terminato a causa di un segnale non gestito\n");
  if(WIFSIGNALED(childstatus))
    printf("Child terminato a causa del segnale %d non gestito\n", WTERMSIG(childstatus));
  
  
  printf("Codice di uscita del figlio: %d\nCiao!\n", WEXITSTATUS(childstatus));
  
  return 0;  
} 