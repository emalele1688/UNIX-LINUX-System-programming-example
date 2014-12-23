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

#define PORTN		20000
#define CL_PORTN	20005


int main(int argc, char *argv[])
{
  struct sockaddr_in srv, claddr;
  struct in_addr inp;
  socklen_t claddr_len = 0;
  int ds_sock;
  char buffer[1024];
  
  memset(&srv, 0, sizeof(srv));
  memset(&claddr, 0, sizeof(claddr));
  memset(buffer, 0, 1024);
  
  if(inet_aton("127.0.0.1", &inp) == 0)
  {
    fprintf(stderr, "Invalid address\n");
    exit(-1);
  }
  
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
  
  claddr.sin_family = AF_INET;
  claddr.sin_port = htons(CL_PORTN);
  memcpy(&claddr.sin_addr, &inp, sizeof(inp));
  
  if( connect(ds_sock, (const struct sockaddr*)&claddr, sizeof(claddr)) == -1)
  {
    fprintf(stderr, "%s\n", strerror(errno));
    exit(-1);
  }
  
  while(1)
  {
    if(read(ds_sock, &buffer, 512) == -1)
    {
      fprintf(stderr, "%s\n", strerror(errno));
      exit(-1);
    }
    
    if(strncmp(buffer, "quit", 4) == 0)
      exit(0);
    
    printf("%s:%d => %s\n", inet_ntoa(claddr.sin_addr), ntohs(claddr.sin_port), buffer);
    
    memset(buffer, 0, sizeof(buffer));
    
    printf("Scrivi qualcosa => ");
    fgets(buffer, 256, stdin);
    if(write(ds_sock, buffer, 512) == -1)
    {
      fprintf(stderr, "%s\n", strerror(errno));
      exit(-1);
    }
          
    if(strncmp(buffer, "quit", 4) == 0)
      exit(0);
        
    memset(buffer, 0, sizeof(buffer));
  }
  
  return 0;
}
