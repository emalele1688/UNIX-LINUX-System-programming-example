#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>

#include "mstreamer.h"


#define PORTN		2050
#define MAX_CONNECTION	3
#define MESSAGE_LENGHT 	256


int server_socket = 0;
int client_socket = 0;


void server_interrupt(int sig)
{
  printf("Chiusura server in corso\n");
  if(server_socket != 0)
    close(server_socket);
  if(client_socket != 0)
    close(client_socket);
  
  exit(EXIT_SUCCESS);
}

void server_bkpipe(int sig)
{
  printf("Il client ha chiuso la connessione\n");
}

void server_task(unsigned short int portn)
{
  signal(SIGINT, &server_interrupt);
  signal(SIGPIPE, &server_bkpipe);
  
  //Apre una socket(dominio, tipo, protocollo) e ne ritorna il descrittore al file. IPPROTO_TCP potrebbe essere omesso poichè la coppia AF_INET e SOCK_STREAM specifica già l'uso del protocollo tcp
  //Dominio: Famiglia del protocollo su livello di rete da utilizzare; AF_INET = IPV4
  //Tipo: Il tipo di protocollo a livello di rete tra quelli della famiglia scelta; SOCK_STREAM = TCP nella famiglia di IPV4
  server_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  if(server_socket == -1)
  {
    printf("%s\n", strerror(errno));
    exit(EXIT_FAILURE);
  }
  
  //Ora specifichiamo gli indirizzi che ci interessa prendere in considerazione
  //Tutte le strutture relative agli indirizzamenti iniziano per sockaddr
  //sockaddr_in: E' la struttura per l'IPV4
  struct sockaddr_in myaddr;
  memset(&myaddr, 0, sizeof(myaddr));
    
  //Famiglia del protocollo in uso sulla socket
  myaddr.sin_family = AF_INET;
  //Porta che vogliamo aprire a questa applicazione
  myaddr.sin_port = htons(portn);
  //Indirizzo di connessione (INADDR_ANY specifica "qualsiasi pacchetto proveniente da qualsiasi IP). La sin_addr punta una struttura in_addr contenente solo la struttura in_addr_t
  myaddr.sin_addr.s_addr = htonl(INADDR_ANY);

  //Asseganmo alla server_socket l'indirizzo espresso in myaddr (di tipo sockaddr_in) - la bind prenderebbe un tipo struttura sockaddr, ma fa casting esplicito
  if(bind(server_socket, (void*)&myaddr, sizeof(myaddr)) == -1)
  {
    printf("%s\n", strerror(errno));
    close(server_socket);
    exit(EXIT_FAILURE);
  }
  
  //Imposta a 3 il numero di connessioni pendenti(massime connessioni accettabili in ingresso)
  if(listen(server_socket, MAX_CONNECTION) == -1)
  {
    printf("%s\n", strerror(errno));
    exit(EXIT_FAILURE);
  }  
  
  //sockaddr: Conterrà l'indirizzi e la famiglia di protocollo usata dal client (prelevata tramite accept)
  struct sockaddr_in clientaddr;
  while(1)
  {
    memset(&clientaddr, 0, sizeof(clientaddr));
    //Attendi una connessione entrante nella socket ds_sock; Scarica in clientaddr l'indirizzo del client connesso, in mess_in_size la sua dimensione. La chiamata è bloccante 
    //Se non ci sono connessioni entranti metti il processo in attesa
    //Se la connessione è stata effettuata in precedenza accettala
    socklen_t mess_in_size = sizeof(clientaddr);
    client_socket = accept(server_socket, (void*)&clientaddr, &mess_in_size);
    if(client_socket == -1)
    {
      //Se accept si interrompe per ricezione di un segnale o disconnesione prematura del client allora rientra in accept
      if(errno == EINTR || errno == ECONNABORTED)
	continue;
      else
	printf("%s\n", strerror(errno));
      
      close(server_socket);
      exit(EXIT_FAILURE);
    }

    //clientaddr.sin_addr contiene l'indirizzo in binario. Va trasformato in stringa
    printf("Connessione stabilita con client di indirizzo: %s:%d\n", inet_ntoa(clientaddr.sin_addr), ntohs(clientaddr.sin_port));
    
    /* SO_KEEPALIVE:
    * Programmo il Kernel in modo che:
    * ogni 2 ore di inattività su questa socket viene inviata un ACK vuota che controlla se la connessione è ancora attiva
    * Il client risponderà con una ACK, altrimenti se la connessione è caduta risponderà con una RST.
    * Se il client non risponde il sisema provvede a ricontattarlo ogni 75 secondi per 9 volte, dopo di che chiude la connessione su questo lato
    */
    int keepalive = 0;
    setsockopt(client_socket, SOL_SOCKET, SO_KEEPALIVE, &keepalive, sizeof(keepalive));
    
    int readret;
    char buffer[256];
    do
    {
     /* Leggi la stringa del client sul descrittore della socket, torna 0 se il client ha chiuso la sua socket
      * Se il client ha chiuso la socket mi ha inviato una FIN, quindi il sistema ha proveduto a far in modo che la read tornasse 0
      * (ha chiuso l'ingresso della socket su questa macchina)
      * ATTENZIONE: se il client cede senza chiudere le socket non mi invia una FIN ed io resto bloccato sulla read
      */
      memset((void*)buffer, 0x0, MESSAGE_LENGHT);
      readret = receiveData(client_socket, buffer, MESSAGE_LENGHT);
      if(readret == 0)
      {
	printf("Il client di indirizzo %s ha chiuso la connessione\n", inet_ntoa(clientaddr.sin_addr));
	break;
      }
      if(readret == -1)
	break;
      
      printf("Il client scrive: %s\n", buffer);
    } 
    while(strncmp(buffer, "quit", 4) != 0);
    
    if(close(client_socket) == -1)
      printf("%s\n", strerror(errno));
    
    client_socket = 0;
  }
}


int main(int argn, char* args[])
{
  printf("Benvenuti!\n");
  server_task(PORTN);
  return 0;
}