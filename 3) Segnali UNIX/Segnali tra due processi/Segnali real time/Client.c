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
  printf("Chiusura client\n");
  close(ds_pipe);
}

void alrm_handler(int sig)
{
  printf("Timeout del client\n");
  close_server();
  exit(0);
}

int main(int argn, char* args[])
{  
  ds_pipe = open("pipe", O_RDWR);
  if(ds_pipe == -1)
  {
    printf("%s\n", strerror(errno));
    return -1;
  }
  
  // 1) Imposto un timer per evitare di rimanere bloccato in read
  struct sigaction alrmsig;
  memset(&alrmsig, 0, sizeof(alrmsig));
  sigemptyset(&alrmsig.sa_mask);
  alrmsig.sa_handler = alrm_handler;
  sigaction(SIGALRM, &alrmsig, NULL);
  alarm(30);
  
  pid_t srvid;
  //In tal modo un segnale può interrompere la syscall
  if(read(ds_pipe, &srvid, sizeof(srvid)) == -1)
  {
    printf("%s\n", strerror(errno));
    close_server();
    return -1;
  }
  
  // 2) Invio il segnale ed il messaggio al server (usando le code di segnali)
  //Messaggio da inviare:
  union sigval info;
  info.sival_int = 12;
  printf("Invio del messaggio\n");
  if(sigqueue(srvid, SIGUSR1, info) == -1)
    printf("%s\n", strerror(errno));
    
  
  close_server();

  return 0;
}