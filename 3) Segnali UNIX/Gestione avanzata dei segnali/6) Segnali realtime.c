#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <errno.h>

void signal_handler(int sig, siginfo_t* sinfo, void* context)
{
  printf("Signal messagge: %s\n", sinfo->si_value);
}


int main(int argn, char* args[])
{
  /* Installo un gestore per SIGUSR1 */
  struct sigaction sig, old_act;
  sigset_t sigstp;
  
  sigemptyset(&sig.sa_mask);
  sig.sa_flags = SA_SIGINFO;
  sig.sa_sigaction = signal_handler;
  
  sigaction(SIGUSR1, &sig, NULL);
  
  /* Blocco il segnale SIGUSR1 per dimostrazione che i segnali realtime sono cumolativi */
  sigemptyset(&sigstp);
  sigaddset(&sigstp, SIGUSR1);
  sigprocmask(SIG_BLOCK, &sigstp, NULL);
  
  /* Invio i segnali realtime */
  union sigval info1;
  info1.sival_ptr = "Messaggio 1";
  if(sigqueue(getpid(), SIGUSR1, info1) == -1)
    printf("%s\n", strerror(errno));
  
  sigprocmask(SIG_UNBLOCK, &sigstp, NULL);
  
  return 0;
}