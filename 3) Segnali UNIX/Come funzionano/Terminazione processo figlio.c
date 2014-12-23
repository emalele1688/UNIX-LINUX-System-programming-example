#include <stdio.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h> 



void gestione_terminazione_figlio(int sig, siginfo_t* sinfo, void* context)
{
  printf("Processo figlio terminato: uscita dallo stato di zombie solo dopo la wait%d\n", sinfo->si_pid);
}

void childProccess(int sl)
{
  printf("Child: I'm alive!\n");
  
  sleep(sl);

  printf("Uscita del processo figlio\n");
  
  exit(sl);
}

int main(int arg, char* args[])
{ 
  char scelta;
  printf("Vuoi ignorare la terminazione del processo figlio (s/n)\n");
  scanf("%c", &scelta);
 
  if(scelta == 's')
    signal(SIGCHLD, SIG_IGN);
  else
  {
    sigset_t set;
    struct sigaction sigact;
    sigemptyset(&set); 
    sigaddset(&set, SIGCHLD);
    
    sigact.sa_mask = set;
    sigact.sa_flags = SA_SIGINFO;
    sigact.sa_sigaction = gestione_terminazione_figlio;
    
    sigaction(SIGCHLD, &sigact, NULL);
  }
 
  pid_t child = fork();
  if(child == 0)
    childProccess(1);
  child = fork();
  if(child == 0)
    childProccess(2);
  
  int status = 0;
  int res = 1;
  while(res > 0)
  {
    res = waitpid(-1, &status, 0);
    printf("Stato terminazione processo figlio: %d\n", status);
  }
  
  printf("Uscita processo padre \n");
  
  return 0;
}
