#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <arpa/inet.h>
#include <netinet/in.h>


#define PORTN	20000


int main(int argc, char *argv[])
{
  struct sockaddr_in srv, claddr;
  socklen_t claddr_len = 0;
  int ds_sock;
  char buffer[1024];
  
  memset(&srv, 0, sizeof(srv));
  memset(&claddr, 0, sizeof(claddr));
  memset(buffer, 0, 1024);
  
  if( (ds_sock = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
  {
    fprintf(stderr, "%s\n", strerror(errno));
    exit(-1);
  }
  
  srv.sin_family = AF_INET;
  srv.sin_port = htons(PORTN);
  srv.sin_addr.s_addr = htonl(INADDR_ANY);
  
  if(bind(ds_sock, (const struct sockaddr*)&srv, sizeof(srv)) == -1)
  {
    fprintf(stderr, "%s\n", strerror(errno));
    exit(-1);
  }
  
  while(1)
  {
    if(recvfrom(ds_sock, &buffer, 1024, 0, (struct sockaddr*)&claddr, &claddr_len) == -1)
    {
      fprintf(stderr, "%s\n", strerror(errno));
      exit(-1);
    }
    
    if(strncmp(buffer, "quit", 4) == 0)
      exit(0);
    
    printf("%s:%d => %s\n", inet_ntoa(claddr.sin_addr), ntohs(claddr.sin_port), buffer);
  }
  
  return 0;
}
