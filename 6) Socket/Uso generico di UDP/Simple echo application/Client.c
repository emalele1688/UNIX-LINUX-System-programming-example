#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <netinet/in.h>
#include <signal.h>
#include <pthread.h>
#include <stdio.h>
#include <time.h>

#define SERVERPORT 1999
#define MESGSIZE 256

char* receivetime()
{
  int ds_sock = socket(AF_INET, SOCK_DGRAM, 0);
  if(ds_sock == -1)
  {
    printf("Impossibile creare un socket ora\n");
    exit(1);
  }
  
  struct sockaddr_in addr;
  memset(&addr, 0, sizeof(addr));
    
  addr.sin_family = AF_INET;
  addr.sin_port = htons(SERVERPORT);
  
  //Pacchetto UDP di richiesta
  if( sendto(ds_sock, NULL, 0, 0, (struct sockaddr *)&addr, sizeof(addr)) < 0)
  {
    printf("Impossibile inviare la richiesta al server daytime\n");
    exit(1);
  }
  
  char buffer[MESGSIZE];
  //Ascolto la ricezione
  if( recvfrom(ds_sock, buffer, MESGSIZE, 0, NULL, NULL) < 0)
  {
    printf("Impossibile ricevere la risposta dal server daytime\n");
    exit(1);
  }
  
  return buffer;
}

int main()
{
  printf("Ricevo la data: %s\n", receivetime());
  return 0;
}
