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

void sighup(int sig)
{
  FILE* fd = fopen("sighup", "w");
  fprintf(fd, "SIGHUP");
  fclose(fd);
}

void childExecute(void)
{
  printf("Ciao, sono il processo figlio con id: %d\n", getpid());
  sleep(10);
  //Possiede il terminale del padre poichè appartiene alla stessa sessione
}

int main(int argn, char* args[])
{
  pid_t chpid;
  int childstatus;
  
  signal(SIGCHLD, &sigchld);
  signal(SIGHUP, &sighup);
  
  printf("Pid del processo: %d\nCreo un processo figlio\n", getpid());
  chpid = fork();
  if(chpid == 0)
  {
    childExecute();
    exit(0);
  }
  else if(chpid == -1)
  {
    printf("%s\n", strerror(errno));
    exit(1);
  }
  
  //Se tolgo la wait il figlio viene ereditato da init, quindi la sua sessione corrisponderà a quella di init. A questo punto alla chiusura del terminale non sarà inviata alcuna SIGHUP
  wait(0);
  printf("Terminazione del padre\n");

  /* REMENBER: Se chiudo il terminale prima ancora che il padre termina, il sistema invia una SIGHUP a tutti i processi figli DELLA STESSA SESSIONE del padre
   * Segnale SIGHUP: Inviato dal sistema alla chiusura di un terminale a tutti i processi ad esso collegati.
   */
  
  return 0;  
} 
 
