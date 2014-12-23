#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <signal.h>
#include <stdio.h>
#include <time.h>

#define MESGSIZE 256
#define PORTN 1999

int ds_sock;

void termina()
{
  close(ds_sock);
  exit(0);
}

void server(short porta)
{
  ds_sock = socket(AF_INET, SOCK_DGRAM, 0);
  
  struct sockaddr_in addr, claddr;
  memset(&addr, 0x0, sizeof(addr));
  memset(&claddr, 0x0, sizeof(claddr));
  
  if(ds_sock == -1)
  {
    printf("Impossibile aprire una socket\n");
    exit(1);
  }
  
  addr.sin_family = AF_INET;
  addr.sin_port = htons(porta);
  addr.sin_addr.s_addr = htonl(INADDR_ANY);
  
  if(bind(ds_sock, (struct sockaddr *)&addr, sizeof(addr)) == -1)
  {
    printf("Impossibile associare gli indirizzi specificati alla socket\n");
    exit(1);
  }
    
  time_t timeval;
  //LEN VA INIZIALIZZATOOOOOOOOOOOOOOOO
  int len = sizeof(struct sockaddr_in);
  char buffer[MESGSIZE];
  memset(buffer, 0x0, MESGSIZE);
  
  while(1)
  {
    if( recvfrom(ds_sock, buffer, MESGSIZE, 0, &claddr, &len) < 0)
      printf("Non è stato possibile ricevere dei dati da un client che ha tentato l'invio\n");
    
    printf("Richiesta presso: %s:%d\n", inet_ntoa(claddr.sin_addr), ntohs(claddr.sin_port));
    timeval = time(NULL);
    snprintf(buffer, sizeof(buffer), "%.24s\r\n", ctime(&timeval));
    
    if( sendto(ds_sock, buffer, strlen(buffer), 0, (struct sockaddr_in *)&claddr, sizeof(claddr)) < 0)
      printf("Impossibile inviare a il client %s:%i la data richiesta!\n", inet_ntoa(claddr.sin_addr), ntohs(claddr.sin_port));
  }
}

int main()
{
  server(PORTN);
  return 0;
}
