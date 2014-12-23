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
  struct sockaddr_in srv, clt;
  struct in_addr inp;
  int ds_sock;
  char message_buffer[256];
  
  memset(&srv, 0, sizeof(srv));
  memset(&clt, 0, sizeof(clt));
  memset(message_buffer, 0, sizeof(message_buffer));
    
  if( (ds_sock = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
  {
    fprintf(stderr, "%s\n", strerror(errno));
    exit(-1);
  }
  
  clt.sin_family = AF_INET;
  clt.sin_port = htons(CL_PORTN);
  
  if(bind(ds_sock, (const struct sockaddr*)&clt, sizeof(clt)) == -1)
  {
    fprintf(stderr, "%s\n", strerror(errno));
    exit(-1);
  }
  
  if(inet_aton("127.0.0.1", &inp) == 0)
  {
    fprintf(stderr, "Invalid address\n");
    exit(-1);
  }
  
  srv.sin_family = AF_INET;
  srv.sin_port = htons(PORTN);
  memcpy(&srv.sin_addr, &inp, sizeof(inp));
  
  if( connect(ds_sock, (const struct sockaddr*)&srv, sizeof(srv)) == -1)
  {
    fprintf(stderr, "%s\n", strerror(errno));
    exit(-1);
  }
  
  while(1)
  {
    printf("Scrivi qualcosa => ");
    fgets(message_buffer, 256, stdin);
    /* If there is no opened socket in the server host for our port number the first write sends successful the data, but the server doesn't has any opened socket at this port number. 
     * So it sends an ICMP message to the client and a following read or write operation of the client to this socket failed with ECONNREFUSED error
     */
    if(write(ds_sock, message_buffer, 512) == -1)
    {
      fprintf(stderr, "%s\n", strerror(errno));
      exit(-1);
    }
          
    if(strncmp(message_buffer, "quit", 4) == 0)
      exit(0);
        
    memset(message_buffer, 0, sizeof(message_buffer));
    
    if(read(ds_sock, &message_buffer, 512) == -1)
    {
      fprintf(stderr, "%s\n", strerror(errno));
      exit(-1);
    }
    
    if(strncmp(message_buffer, "quit", 4) == 0)
      exit(0);
    
    printf("%s:%d => %s\n", inet_ntoa(srv.sin_addr), ntohs(srv.sin_port), message_buffer);
    
    memset(message_buffer, 0, sizeof(message_buffer));
  }
  
  return 0;
}
