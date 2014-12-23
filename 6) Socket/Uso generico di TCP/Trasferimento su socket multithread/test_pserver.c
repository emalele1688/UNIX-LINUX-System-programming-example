#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>

#include "concurrent_streamer.h"
#include "streamer.h"


#define SERVER_PORTN	2051

#define CLOSE_ALL_PIPE() 	\
{				\
  close(client_sock); 		\
  close(server_sock); 		\
  close(new_file); 		\
}


int server_sock;
int new_file;
int client_sock;


void receive(void)
{
  int ret;
  cstreamer_t* cst;
  
  size_t fileLenght;
  read_block(client_sock, &fileLenght, sizeof(size_t));
  printf("Dimensione del file da ricevere %lu byte\n", fileLenght);
  
  new_file = open("new", O_CREAT|O_RDWR, 0666);
  if(new_file == -1)
  {
    printf("%s\n", strerror(errno));
    close(client_sock);
    close(server_sock);
    exit(-1);
  }
  
  if((ret = cstreamer_init(&cst)) != 0)
  {
    printf("%s\n", strerror(ret));
    CLOSE_ALL_PIPE();
    unlink("new");
    exit(-1);
  }
    
  if((ret = cstreamer_start(cst, client_sock, new_file, fileLenght)) != 0)
  {
    printf("%s\n", strerror(ret));
    CLOSE_ALL_PIPE();
    unlink("new");
    exit(-1);
  }    
  
  fileLenght = 0;
  if((ret = cstreamer_wait_transferer(cst, &fileLenght)) != 0)
    printf("%s\n", strerror(errno));
  
  printf("%lu\n", fileLenght);
}

void seg_handler(int sig)
{
  printf("Errore di segmentazione\n");
  CLOSE_ALL_PIPE();
}

void bpipe_handler(int sig)
{
  printf("Broken pipe\n");
  CLOSE_ALL_PIPE();
  exit(-1);
}

void int_handler(int sig)
{
  printf("Interrupt\n");
  CLOSE_ALL_PIPE();
  exit(0);
}


int main(int argn, char* args[])
{
  signal(SIGINT, int_handler);  
  signal(SIGPIPE, bpipe_handler);
  signal(SIGSEGV, seg_handler);
  
  socklen_t cls;
  struct sockaddr_in maddr;
  struct sockaddr_in claddr;
  
  server_sock = socket(AF_INET, SOCK_STREAM, 0);
  if(server_sock == -1)
  {
    printf("%s\n", strerror(errno));
    exit(-1);
  }
  
  maddr.sin_family = AF_INET;
  maddr.sin_port = htons(SERVER_PORTN);
  maddr.sin_addr.s_addr = htonl(INADDR_ANY);
  
  if(bind(server_sock, (void*)&maddr, sizeof(maddr)) == -1)
  {
    printf("%s\n", strerror(errno));
    close(server_sock);
    exit(-1);
  }
  
  listen(server_sock, 3);
  
  cls = sizeof(claddr);
  client_sock = accept(server_sock, (void*)&claddr, &cls);
  if(client_sock == -1)
  {
    printf("%s\n", strerror(errno));
    close(server_sock);
    exit(-1);
  }
  
  printf("Accettata connessione con %s\n", inet_ntoa(claddr.sin_addr));
  
  receive();
      
  return 0;
}
