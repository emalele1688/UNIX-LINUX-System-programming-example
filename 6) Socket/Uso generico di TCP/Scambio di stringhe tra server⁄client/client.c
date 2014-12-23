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

#define SERVER_PORT 2050
#define MESSAGE_LENGHT 256

int client_socket = 0;

void connessione_senza_lettore(void)
{
  printf("La connessione è stata chiusa sull'altro lato\n");
}

void term(void)
{
  printf("Segnale di terminazione\n");
  if( close(client_socket) == -1 )
    printf("Impossibile chiudere il client %s\n", strerror(errno));
  printf("Exit\n");
  exit(0);
}

int main(int arg, char* args[])
{
  signal(SIGPIPE, (void*)&connessione_senza_lettore);
  signal(SIGINT, (void*)&term);
  
  //client_socket è la socket dell'applicazione
  int client_socket, ret;
  //Contiene gli indirizzi (IP e porto) dell'applicazione server
  struct sockaddr_in myaddr, claddr;
  memset(&myaddr, 0, sizeof(myaddr));
  memset(&claddr, 0, sizeof(claddr));
  
  //Creiamo una socket TCP------------------------------------------------
  client_socket = socket(AF_INET, SOCK_STREAM, 0);
  if(client_socket == -1)
  {
    printf("Non è stato possibile aprire una socket %s\n", strerror(errno));
    exit(1);
  }
  
  claddr.sin_family = AF_INET;
  claddr.sin_port = htons(11000);
  bind(client_socket, (void*)&claddr, sizeof(claddr));
  
  //Connettiamoci al server----------------------------------------------
  
  //Specifichiamo a sockaddr_in la famiglia di protocolli usata dal server
  myaddr.sin_family = AF_INET;
  //Specifichiamo il porto dell'applicazione a lato server
  myaddr.sin_port = htons(SERVER_PORT);
  
  //Connettiamoci all'applicazione su server specificato da myaddr (Esegue l'handshake TCP)
  ret = connect(client_socket, (void*)&myaddr, sizeof(myaddr));
  if(ret == -1)
  {
    printf("Connessione fallita: %s\n", strerror(errno));
    close(client_socket);
    exit(1);
  }
  

  
  printf("Siamo connessi\n");
  
  /* Scenario interessante:
   * Se il server chiude la connessione invierà una FIN al client su questo porto (il porto di questa applicazione).
   * La connessione dal lato server sarà allora chiusa ed il client non se ne accorge della FIN poichè non è dentro la write.
   * Sul mio lato la connessione invece sarà ancora aperta, allora potro scrivere dinuovo sulla socket,
   * ed il pacchetto giungerà al server che risponderà con ACK-FIN (connessione chiusa):
   * pacchetto giunto a destinazione con successo ma il porto specificato ha chiuso la connessione (flag FIN attivo).
   * A questo punto su questa macchina la socket verso il server sarà data per chiusa a metà,
   * ed al successivo tentativo di scrittura su quella socket il server invierà una RST dove il mio S.O. invierà una SIGPIPE all'applicazione server.
   * Nota: Alla ricezione di una RST su una il S.O. invia una SIGPIPE all'applicazione di quella porta
   * 
   * FIN = Connessione chiusa
   * RST = Socket non esistente
   */

  size_t length;
  char buffer[MESSAGE_LENGHT];
  do
  {
    printf("Cosa dici al server?(quit per uscire)\n");
    fgets(buffer, MESSAGE_LENGHT, stdin);
    //Scrive sul descrittore client_socket, invia quindi la stringa al server (descrittore al file socket)
    length = sendData(client_socket, buffer, MESSAGE_LENGHT);
    if(length == -1)
    {
      printf("Errore in scrittura sul socket %s\n", strerror(errno));
      close(client_socket);
      return -1;
    }
  }
  while(strncmp(buffer, "quit", 4) != 0);
  
  /*Chiusura del socket del client
   * Come avviene la chiusura della connessione:
   * viene inviato al Server un pacchetto TCP con flag FIN ed attendo la notifica di avvenuta ricezione da parte del server (ACK)
   * Il S.O. su questo lato (client) dealloca tutte le risorse per chiudere la connessione
   * Dal lato server la connessione resterà ancora aperta, se il server cercherà di mettersi in comunicazione con me la mia macchina inviera una RST.
   */
  if( close(client_socket) == -1 )
    printf("Impossibile chiudere il client %s\n", strerror(errno));
  
  return 0;
}

