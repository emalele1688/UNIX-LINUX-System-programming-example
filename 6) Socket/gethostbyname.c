#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>


int main(int s, char* arg[])
{
  struct hostent *host;
  struct in_addr host_addr;
  
  if(s != 2)
  {
    printf("Passare nome host come input\n");
    exit(0);
  }
  
  printf("Elaboro %s\n", arg[1]);
  
  host = gethostbyname(arg[1]);
  if(host == NULL)
  {
    printf("Nome host non valido\n");
    exit(0);
  }
  
  //Scarico la stringa degli indirizzi dell'host nella host_addr (struttura per gli indirizzi IPV4)
  memcpy(&host_addr, host->h_addr, 4);
  
  printf("Nome host: %s\n", host->h_name);
  printf("Address type: %d\n", host->h_addrtype);
  printf("Lunghezza address: %d\n", host->h_length);
  //Convertirò in char* il contenuto del campo in_addr_t in in_addr
  printf("IP: %s\n", inet_ntoa(host_addr));
  
  return 0;
}

