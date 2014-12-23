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

#include "streamer.h"


#define SERVER_PORTN	2051
#define CLOSE_ALL_PIPE()	\
{				\
  close(output_file);		\
  close(ds_sock);		\
}


int ds_sock;
int output_file;


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


void startTransferer(const char* filePath)
{
  cstreamer_t *cstr;
  size_t fileLenght;
  int ret;
  
  output_file = open(filePath, O_CREAT|O_RDWR, 0666);
  if(output_file == -1)
  {
    printf("%s\n", strerror(errno));
    close(ds_sock);
    close(output_file);
    exit(-1);
  }
  
  fileLenght = lseek(output_file, 0, SEEK_END);
  lseek(output_file, 0, SEEK_SET);
  printf("Dimensione file da inviare %lu\n", fileLenght);
  
  if(write_block(ds_sock, &fileLenght, sizeof(size_t)) < sizeof(size_t))
  {
    printf("%s\n", strerror(errno));
    exit(-1);
  }
  
  void* buffer = malloc(fileLenght);
  if(fileLenght == 0)
  {
    printf("%s\n", strerror(errno));
    exit(-1);
  }
  
  if(read_block(output_file, buffer, fileLenght) < fileLenght)
  {
    printf("%s\n", strerror(errno));
    exit(-1);
  }
  
  if(write_block(ds_sock, buffer, fileLenght) < fileLenght)
  {
    printf("%s\n", strerror(errno));
    exit(-1);
  }
}


int main(int argn, char* args[])
{
  signal(SIGINT, int_handler);  
  signal(SIGPIPE, bpipe_handler);
  signal(SIGSEGV, seg_handler);
  
  struct sockaddr_in maddr;
  
  if(argn < 2)
    exit(0);
  
  memset(&maddr, 0, sizeof(maddr));
  
  output_file = open(args[1], O_RDONLY);
  if(output_file == -1)
  {
    printf("%s: %s\n", args[1], strerror(errno));
    exit(-1);
  }
  
  ds_sock = socket(AF_INET, SOCK_STREAM, 0);
  if(ds_sock == -1)
  {
    printf("socket: %s\n", strerror(errno));
    close(output_file);
    exit(-1);
  }
  
  maddr.sin_family = AF_INET;
  maddr.sin_port = htons(SERVER_PORTN);
  maddr.sin_addr.s_addr = htonl(INADDR_ANY);
  
  if(connect(ds_sock, (void*)&maddr, sizeof(maddr)) == -1)
  {
    printf("connect: %s\n", strerror(errno));
    CLOSE_ALL_PIPE();
    exit(-1);
  }
  
  startTransferer(args[1]);
  
  CLOSE_ALL_PIPE();
  
  return 0;
} 
