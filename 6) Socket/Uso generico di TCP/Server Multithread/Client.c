#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <errno.h>
#include <asm/errno.h>

#include "TrasfererMan.h"

#define SERVERPORT 1912
#define MESGSIZE 256

int ds_sock = 0;

void client_interrupt(int sig)
{
  printf("Interrotto da segnale\n");
  
  close(ds_sock);
  exit(0);
}

void broken_pipe(int sig)
{
  printf("Il client ha chiuso la connessione\n");
}


int main(int s, char* args)
{
  struct sigaction sigint;
  struct sigaction sigpipe;
  
  sigset_t set;
  sigemptyset(&set);
  sigaddset(&set, SIGINT);
  sigaddset(&set, SIGPIPE);
  
  sigint.sa_mask = set;
  sigint.sa_handler = client_interrupt;
  sigint.sa_flags = 0;
  sigaction(SIGINT, &sigint, NULL);
  
  sigpipe.sa_mask = set;
  sigpipe.sa_handler = broken_pipe;
  sigpipe.sa_flags = 0;
  sigaction(SIGPIPE, &sigpipe, NULL);
  
  struct sockaddr_in sraddr;
  ds_sock = socket(AF_INET, SOCK_STREAM, 0);
  
  sraddr.sin_family = AF_INET;
  sraddr.sin_port = htons(SERVERPORT);
  
  printf("Connessione in corso verso la porta %i in localhost\n", ntohs(sraddr.sin_port));
  int ret = connect(ds_sock, (void*)&sraddr, sizeof(sraddr));
  if(ret == -1)
  {
    printf("Connessione fallita verso server 1911 si localhost\n");
    close(ds_sock);
    exit(1);
  }
  
  char messagge[MESGSIZE];
  memset(messagge, 0x0, MESGSIZE);
 
  /* Ricevo 4 messaggi dal server per simulare casi in qui:
   * ricevendo messaggi da un server, se il client abbandona la connessione, ad una write del server 
   * questa macchina invierebbe una RST e il server ad una successiva write cadrebbe per una SIGPIPE
   * MA ciò non accade perchè ho impostato il server che cattura le SIGPIPE.
   */
  do
  {
    printf("Messaggi:\n");
    int i = 0;
    while(i < 4)
    {
      ret = receiveData(ds_sock, messagge, MESGSIZE);
      if(ret == -1)
      {
	printf("Errore\n");
	close(ds_sock);
	return -1;
      }
      else if(ret == 0)
      {
	printf("Il server ha chiuso la connessione\n");
	close(ds_sock);
	return 0;
      }
      
      printf("%s\n", messagge);
      
      i++;
    }
    
    printf("Invia tu un messaggio\n");
    fgets(messagge, MESGSIZE, stdin);
    if( sendData(ds_sock, messagge, MESGSIZE) == -1 )
    {
      printf("Errore nel spedire messaggi al server\n");
      close(ds_sock);
      return -1;
    }
  }
  while(strncmp(messagge, "quit", 4) != 0);
  
  if(close(ds_sock) == -1)
    printf("Problemi di chiusura della socket\n");
}