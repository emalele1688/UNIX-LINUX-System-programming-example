#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>


#include "mstreamer.h"


#define PORTN		2080
#define MAX_CONNECTION	3
#define MESSAGE_LENGHT 	256


int server_socket = 0;

void child_term(int sig)
{
  int errno_save = errno;
  int status = 0;
  pid_t pid;
  
  do
  {
    errno = 0;
    pid = waitpid(WAIT_ANY, &status, WNOHANG);
    if(pid > 0)
      printf("Terminato il processo %d\n", pid);
  } 
  while(pid > 0);
  
  errno = errno_save;
}

void server_pause(int sig)
{
  printf("Interruzione del server dall'utente\n");
}

void server_term()
{
  int status;
  pid_t pid = 0;
  
  printf("Chiusura del server in corso\n");
  
  do
  {
    pid = waitpid(WAIT_ANY, &status, 0);
  } 
  while(pid > 0);
  
  close(server_socket);
  exit(EXIT_SUCCESS);
}

void server_task(int client_socket, struct sockaddr_in* clientaddr)
{
  char buffer[MESSAGE_LENGHT];  
  do
  {
    memset((void*)buffer, 0x0, MESSAGE_LENGHT);
    int readret = receiveData(client_socket, buffer, MESSAGE_LENGHT);
    if(readret == 0)
    {
      printf("Il client di indirizzo %s ha chiuso la connessione\n", inet_ntoa(clientaddr->sin_addr));
      break;
    }
    if(readret == -1)
      break;
    
    printf("Il client scrive: %s\n", buffer);
  } 
  while(strncmp(buffer, "quit", 4) != 0);
  
  close(client_socket);
  exit(EXIT_SUCCESS);
}

void server_execute(void)
{
  struct sigaction term;
  struct sigaction pause;
  struct sigaction clchild;
  
  struct sockaddr_in myaddr;
  struct sockaddr_in clientaddr;
  int client_socket = 0;
    
  memset(&term, 0, sizeof(term));
  memset(&pause, 0, sizeof(pause));
  memset(&clchild, 0, sizeof(clchild));
  //SIGINT-SIGTERM
  sigemptyset(&term.sa_mask);
  sigaddset(&term.sa_mask, SIGINT);
  sigaddset(&term.sa_mask, SIGTERM);
  //Se sto terminando non entrare in pausa
  sigaddset(&term.sa_mask, SIGSTOP);
  //Se sto terminando non entrare in child_term
  sigaddset(&term.sa_mask, SIGCHLD);
  term.sa_handler = &server_term;
  sigaction(SIGINT, &term, NULL);
  sigaction(SIGTERM, &term, NULL);
  //SIGSTP
  sigemptyset(&pause.sa_mask);
  sigaddset(&pause.sa_mask, SIGCHLD);
  pause.sa_handler = &server_pause;
  sigaction(SIGSTOP, &pause, NULL);
  //SIGCHLD - IMPORTANTE
  sigemptyset(&clchild.sa_mask);
  sigaddset(&clchild.sa_mask, SIGINT);
  sigaddset(&clchild.sa_mask, SIGTERM);
  clchild.sa_handler = &child_term;
  sigaction(SIGCHLD, &clchild, NULL);
  
  server_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  if(server_socket == -1)
  {
    printf("%s\n", strerror(errno));
    exit(EXIT_FAILURE);
  }

  memset(&myaddr, 0, sizeof(myaddr));

  //Famiglia del protocollo in uso su la socket
  myaddr.sin_family = AF_INET;
  //Porta che vogliamo aprire a questa applicazione
  myaddr.sin_port = htons(PORTN);
  //Indirizzo di connessione (INADDR_ANY specifica "qualsiasi pacchetto proveniente da qualsiasi IP). La sin_addr punta una struttura in_addr contenente solo la struttura in_addr_t
  myaddr.sin_addr.s_addr = htonl(INADDR_ANY);

  if(bind(server_socket, (void*)&myaddr, sizeof(myaddr)) == -1)
  {
    printf("%s\n", strerror(errno));
    close(server_socket);
    exit(EXIT_FAILURE);
  }

  if(listen(server_socket, MAX_CONNECTION) == -1)
  {
    printf("%s\n", strerror(errno));
    close(server_socket);
    exit(EXIT_FAILURE);
  }
  
  socklen_t clstsize = sizeof(clientaddr);
  int ret;
  
  while(1)
  {    
    memset(&clientaddr, 0, sizeof(clientaddr));
    client_socket = accept(server_socket, (void*)&clientaddr, &clstsize);
    if(client_socket == -1)
    {
      if(errno == EINTR || errno == ECONNABORTED)
	continue;
      
      printf("%s\n", strerror(errno));
      server_term();
    }

    printf("Connessione stabilita con client di indirizzo: %s\n",  inet_ntoa(clientaddr.sin_addr));

    ret = fork();
    if(ret == -1)
    {
      printf("Impossibile gestire il client %s: %s\n",  inet_ntoa(clientaddr.sin_addr), strerror(errno));
      close(client_socket);
      continue;
    }
    else if(ret == 0)
      server_task(client_socket, &clientaddr);
  }
}

int main(int argn, char* args[])
{
  server_execute();
  return 0;
}