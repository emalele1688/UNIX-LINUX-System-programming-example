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
  printf("Anche se mio padre è uscito, io resto dentro il sistema.. mi adotta init\n");
  sleep(5);
  //Possiede il terminale del padre poichè appartiene alla stessa sessione
  
  return 1;
}

int main(int argn, char* args[])
{
  pid_t chpid;
  int childstatus;
  
  //Il processo padre non riceverà mai il segnale, ma lo riceverà init poichè lo adotta
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

  return 0;  
} 
