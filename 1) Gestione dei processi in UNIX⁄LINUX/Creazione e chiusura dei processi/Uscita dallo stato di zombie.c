#include <unistd.h> 
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <string.h>
#include <errno.h>


#define SLEEP_TIME 10


void childTerm(int sig)
{
  /* Lo stato dell'errno viene modificato nel gestore del segnale poichè invocherò una syscall
   * Rischio quindi di perderlo se il segnale viene ricevuto subito dopo una syscall 
   * es. potrebbe essere ricevuto a cavallo di una syscall e la relativa printf che stampa il tipo di errore
   */
  int errno_save = errno;
  int status = 0;
  pid_t pid;
  
  //Terminiamo tutti i processi zombie
  do
  {
    errno = 0;
    /* WAIT_ANY: Attendi per un qualsiasi processo figlio
     * WNOHANG: Torna se non ci sono processi zombie (quindi non attende la terminazione dei processi figlio in esecuzione)
     */
    pid = waitpid(WAIT_ANY, &status, WNOHANG);
    if(pid > 0)
      printf("Terminato il processo %d\n", pid);
  } 
  while(pid > 0);
  
  errno = errno_save;
}


void processOne(void)
{
  printf("Ciao sono pid %d\n", getpid());
  //Simula qualcosa
  sleep(5);
  
  exit(EXIT_SUCCESS);
}

void processTwo(void)
{
  printf("Ciao sono pid %d\n", getpid());
  //Simula qualcosa
  sleep(8);
  
  exit(EXIT_SUCCESS);
}


int main(int argn, char* args[])
{
  int sleepsec = SLEEP_TIME;
  
  /* Installo un gestore di segnale SIGCHLD: Viene lanciato dal kernel verso il processo padre se un processo figlio termina
   * Proseguo poi la normale esecuzione. 
   * Quando uno o più processi figli terminano il sistema mi invia almeno un segnale SIGCHLD (non cumulativo) ed inizia l'esecuzione del gestore che provedde a terminare correttamente il processo figlio.
   */
  signal(SIGCHLD, &childTerm);
  
  if(fork() == 0)
    processOne();
  
  if(fork() == 0)
    processTwo();
  
  //Esecuzione normale (simulata da una sleep) ----------
  //Alla ricezione di un segnale SIGCHLD il processo esce fuori dalla sleep, ma rientra immediatamente
  while(sleepsec > 0)
  {
    sleepsec = sleep(sleepsec);
    if(sleepsec > 0 && errno == EINTR)
    {
      printf("Interruzione del processo padre per gestione di un segnale (sicuramente è terminato un processo figlio)\n");
      continue;
    }
  }
  
  return 0;
}
