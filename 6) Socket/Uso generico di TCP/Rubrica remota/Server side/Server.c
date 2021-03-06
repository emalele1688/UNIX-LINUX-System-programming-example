#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#include "Server.h"
#include "Streamer.h"
#include "ServerProtocol.h"


#define USER_RD		49
#define USER_RDWR	50

static int FLAG_TERM;
static int FLAG_TIMEOUT;
static int FLAG_BPIPE;

typedef struct _Server
{
  AddressBook* addressBook;
  UserBook* userBook;
  int ds_socket;
  int ds_clientSocket;
  int execute;
  struct sockaddr_in serverAddr;
  struct sockaddr_in clientAddr;
} Server;


int Server_service(Server* server);


void Server_term(int sig)
{
  printf("Termination\n");
  FLAG_TERM = 1;
}

void Server_timeout(int sig)
{
  printf("Timeout\n");
  FLAG_TIMEOUT = 1;
}

void Server_brokenPipe(int sig)
{
  printf("Broken pipe\n");
  FLAG_BPIPE = 1;
}

Server* Server_initServer(uint16_t portNumber)
{ 
  FLAG_TERM = 0;
  FLAG_TIMEOUT = 0;
  FLAG_BPIPE = 0;
  
  struct sigaction exitAction;
  struct sigaction oldIntAction;
  struct sigaction oldQuitAction;
  struct sigaction oldTermAction;
  
  sigemptyset(&exitAction.sa_mask);
  
  //Mi devi gestire i tre segnali di uscita in questa routine, se uno di essi si verifica torna 0 e imposta il FLAG_TERM
  //I segnali di uscita sono gestiti dallo stesso handler, se uno di essi si verifica è meglio tener bloccati gli altri due.
  sigaddset(&exitAction.sa_mask, SIGINT);
  sigaddset(&exitAction.sa_mask, SIGQUIT);
  sigaddset(&exitAction.sa_mask, SIGTERM);
  exitAction.sa_flags = 0;
  exitAction.sa_handler = Server_term;
  if(sigaction(SIGINT, &exitAction, &oldIntAction) == -1 || sigaction(SIGQUIT, &exitAction, &oldQuitAction) == -1 || sigaction(SIGTERM, &exitAction, &oldTermAction) == -1)
  {
    fprintf(stderr, "Problemi nell'installare un gestore per i tre segnali di uscita: %s\n", strerror(errno));
    return 0;
  }
  
  Server* server = malloc(sizeof(Server));
  if(server == (void*)0)
  {
    fprintf(stderr, "Impossibile inizializzare il server: %s", strerror(errno));
    return (Server*)0;
  }
  memset(server, 0, sizeof(Server));
  
  server->ds_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  if(server->ds_socket == -1)
  {
    fprintf(stderr, "Impossibile creare una socket per il server %s\n", strerror(errno));
    free(server);
    return (Server*)0;
  }
  
  server->serverAddr.sin_family = AF_INET;
  server->serverAddr.sin_port = htons(portNumber);
  server->serverAddr.sin_addr.s_addr = INADDR_ANY; //Pacchetto proveniente da qualsiasi host
  
  if( bind(server->ds_socket, (struct sockaddr*)(&server->serverAddr), sizeof(struct sockaddr_in)) == -1)
  {
    fprintf(stderr, "Impossibile assegnare gli indirizzi dati alla socket: %s\n", strerror(errno));
    close(server->ds_socket);
    free(server);
    return (Server*)0;
  }
  
  //Ripristino i segnali
  if(sigaction(SIGINT, &oldIntAction, NULL) == -1 || sigaction(SIGQUIT, &oldQuitAction, NULL) == -1 || sigaction(SIGTERM, &oldTermAction, NULL) == -1)
  {
    fprintf(stderr, "Problemi nell'installare un gestore per i tre segnali di uscita: %s\n", strerror(errno));
    return 0;
  }
  
  if(FLAG_TERM == 1)
    return 0;
  
  return server;
}

void Server_setUserBook(Server* server, UserBook* book)
{
  server->userBook = book;
}

void Server_setAddressBook(Server* server, AddressBook* book)
{
  server->addressBook = book;
}

int Server_execute(Server* server)
{ 
  if(FLAG_TERM == 1)
    return 1;
  /* Il server non può iniziare l'esecuzione del servizio se non si sono impostati i corrispettivi 
   * UserBook (registro utenti) e AddressBook (rubrica)
   */
  if(server->userBook == 0 || server->addressBook == 0)
  {
    printf("Non è stata caricata alcuna rubrica e/o elenco di utenti\n");
    return 1;
  }
  
  //Azzero i flag relativi ai segnali (FLAG_TERM deve essere perforza a 0)
  FLAG_TIMEOUT = 0;
  FLAG_BPIPE = 0;
  
  sigset_t blockedSig;
  
  struct sigaction exitAction;
  struct sigaction alrmAction;
  struct sigaction closePipeAction;
  struct sigaction oldIntAction;
  struct sigaction oldQuitAction;
  struct sigaction oldTermAction;
  
  socklen_t addrlen;
  int keepalive = 0;
  
  if( listen(server->ds_socket, MAX_PENDING_CONNECTION) == -1)
  {
    fprintf(stderr, "Impossibile impostare un limite per la coda delle connessioni completate: %s\n", strerror(errno));
    return 1;
  }
  
  sigemptyset(&blockedSig);
  sigemptyset(&exitAction.sa_mask);
  sigemptyset(&alrmAction.sa_mask);
  sigemptyset(&closePipeAction.sa_mask);
 
  //I segnali di uscita sono gestiti dallo stesso handler, se uno di essi si verifica è meglio tener bloccati gli altri due.
  sigaddset(&exitAction.sa_mask, SIGINT);
  sigaddset(&exitAction.sa_mask, SIGQUIT);
  sigaddset(&exitAction.sa_mask, SIGTERM);
  exitAction.sa_flags = 0;
  exitAction.sa_handler = Server_term;
  if(sigaction(SIGINT, &exitAction, &oldIntAction) == -1 || sigaction(SIGQUIT, &exitAction, &oldQuitAction) == -1 || sigaction(SIGTERM, &exitAction, &oldTermAction) == -1)
  {
    fprintf(stderr, "Problemi nell'installare un gestore per i tre segnali di uscita: %s\n", strerror(errno));
    return 0;
  }
  
  closePipeAction.sa_flags = 0;
  closePipeAction.sa_handler = SIG_IGN;
  if(sigaction(SIGPIPE, &closePipeAction, NULL) == -1)
    fprintf(stderr, "Problemi nell'installare un gestore per il segnale SIGPIPE: %s\n", strerror(errno));
    
  alrmAction.sa_flags = 0;
  alrmAction.sa_handler = Server_timeout;
  if(sigaction(SIGALRM, &alrmAction, NULL) == -1)
  {
    fprintf(stderr, "Problemi nell'installare un gestore per il segnale SIGALRM: %s\n", strerror(errno));
    return 1;
  }

  server->execute = 1;
  
  alarm(TIMEOUT_SECOND);
  
  while(server->execute)
  {
    FLAG_TIMEOUT = 0;
    FLAG_BPIPE = 0;
    
    memset(&server->clientAddr, 0, sizeof(struct sockaddr_in));
    addrlen = sizeof(struct sockaddr_in);
    
    if(FLAG_TERM == 1 || FLAG_TIMEOUT == 1)
      break;
    
    printf("Attedo connessione con client\n");
    server->ds_clientSocket = accept(server->ds_socket, (struct sockaddr*)(&server->clientAddr), &addrlen);
    if(server->ds_clientSocket == -1)
    {
      if(errno != EINTR)
      {
	fprintf(stderr, "Il seguente errore è stato riscontrato durante l'attesa di una connessione entrante: %s\n", strerror(errno));
      
	//In alcuni casi potrebbe essere inutile tentare di ricevere altre connessioni, quindi esco:
	if(errno == EBADF || errno == ENOTSOCK || errno == ENOMEM)
	{
	  printf("Impossibile continuare\n");
	  server->execute = 0;
	  break;
	}
      }
      else if(errno == EINTR)
      {
	if(FLAG_TIMEOUT == 1 || FLAG_TERM == 1)
	  break; 
      }

      continue;
    }
    
    //Disabilito il timer
    alarm(0);
    
    printf("Connessione stabilita con client di indirizzo: %s\n", inet_ntoa(server->clientAddr.sin_addr));
    
    /* Durante l'interazione con il client:
     * SIGPIPE: Recettibile
     * SIGALRM: Ignorato
     * SIGTERM: Bloccato
     * SIGTSTP: Bloccato
     */
    
    closePipeAction.sa_handler = Server_brokenPipe;
    if(sigaction(SIGPIPE, &closePipeAction, NULL) == -1)
      fprintf(stderr, "Problemi nell'installare un gestore per il segnale SIGPIPE: %s\n", strerror(errno));
    
    alrmAction.sa_handler = SIG_IGN;
    if(sigaction(SIGALRM, &alrmAction, NULL) == -1)
      fprintf(stderr, "Problemi nell'installare un gestore per il segnale SIGALRM: %s\n", strerror(errno));
    
    sigaddset(&blockedSig, SIGTERM);
    sigaddset(&blockedSig, SIGTSTP);
    sigprocmask(SIG_SETMASK, &blockedSig, NULL);
    
    //Imposto un keepalive di default verso il client
    keepalive = 1;
    setsockopt(server->ds_clientSocket, SOL_SOCKET, SO_KEEPALIVE, &keepalive, sizeof(keepalive));
    
    //Gestisci client
    Server_service(server);
    
    keepalive = 0;
    setsockopt(server->ds_clientSocket, SOL_SOCKET, SO_KEEPALIVE, &keepalive, sizeof(keepalive));
    printf("Disconnessione in corso\n");
    close(server->ds_clientSocket);
    server->ds_clientSocket = 0;
    
    //Rimposto e sblocco tutti i segnali
    
    closePipeAction.sa_handler = SIG_IGN;
    if(sigaction(SIGPIPE, &closePipeAction, NULL) == -1)
      fprintf(stderr, "Problemi nell'installare un gestore per il segnale SIGPIPE: %s\n", strerror(errno));
    
    alrmAction.sa_handler = Server_timeout;
    if(sigaction(SIGALRM, &alrmAction, NULL) == -1)
    {
      fprintf(stderr, "Problemi nell'installare un gestore per il segnale SIGALRM: %s\n", strerror(errno));
      break; 
    }
    
    sigemptyset(&blockedSig);
    sigprocmask(SIG_SETMASK, &blockedSig, NULL);
    
    alarm(TIMEOUT_SECOND);
  }
  
  alarm(0);
  server->execute = 0;
  
  sigemptyset(&blockedSig);
  sigprocmask(SIG_SETMASK, &blockedSig, NULL);
  
  closePipeAction.sa_handler = SIG_IGN;
  if(sigaction(SIGPIPE, &closePipeAction, NULL) == -1)
    fprintf(stderr, "Problemi nell'installare un gestore per il segnale SIGPIPE: %s\n", strerror(errno));
    
  alrmAction.sa_flags = 0;
  alrmAction.sa_handler = SIG_IGN;
  if(sigaction(SIGALRM, &alrmAction, NULL) == -1)
    fprintf(stderr, "Problemi nell'installare un gestore per SIGALRM: %s\n", strerror(errno));
  
  if(sigaction(SIGINT, &oldIntAction, NULL) == -1 || sigaction(SIGQUIT, &oldQuitAction, NULL) == -1 || sigaction(SIGTERM, &oldTermAction, NULL) == -1)
    fprintf(stderr, "Problemi nell'installare un gestore per i tre segnali di uscita: %s\n", strerror(errno));
  
  FLAG_TIMEOUT = 0;
  FLAG_BPIPE = 0;
  
  // 0 = OK
  // 1 = Err
  return 0;
}

int Server_service(Server* server)
{
  packet_login inlog;
  packet_entry outent;
  packet_operation inoper;
  packet_accept acc;
  int ret;
  char priv;
  
  memset(&inlog, 0, sizeof(packet_login));
  memset(&outent, 0, sizeof(packet_entry));
  memset(&inoper, 0, sizeof(packet_operation));
  memset(&acc, 0, sizeof(packet_accept));
  
  //Se non ho ancora accettato il Login faccio in tempo a terminare il Server
  if(FLAG_TERM == 1)
    return 1;
   
  //Leggi login
  if( (ret = readToStream(server->ds_clientSocket, &inlog, sizeof(packet_login))) < sizeof(packet_login))
  {
    if(ret == 0)
      printf("Il client potrebbe aver chiuso la connessione\n");
    else
      printf("Problemi durante la lettura da socket: %s\n", strerror(errno));
    
    return 1;
  }
  
  priv = UserBook_findUser(server->userBook, inlog.user_name, inlog.user_pass);
  acc.byteinfo = priv;
    
  if(priv == 0)
  {
    //invia risposta negativa al login
    printf("Utente %s non accettato\n", inlog.user_name);
    writeToStream(server->ds_clientSocket, &acc, sizeof(packet_accept));
    return 1;
  }
  
  printf("Utente %s accettato con privileggi %d\n", inlog.user_name, priv);

  //Invia risposta positiva al login
  if( writeToStream(server->ds_clientSocket, &acc, sizeof(packet_accept)) < sizeof(packet_accept))
  {
    if(errno == EPIPE)
      printf("L'altro capo della rete potrebbe essere irraggiungibile per la scrittura\n");
    
    printf("Problemi durante la scrittura da socket: %s\n", strerror(errno));
    return 1;
  }
  
  memset(&acc, 0, sizeof(packet_accept));
  
  //Leggi operazione da effettuare
  if( (ret = readToStream(server->ds_clientSocket, &inoper, sizeof(packet_operation))) < sizeof(packet_operation))
  {
    if(ret == 0)
      printf("Il client potrebbe aver chiuso la connessione\n");
    else
      printf("Problemi durante la lettura da socket: %s\n", strerror(errno));
    
    return 1;
  }
  
  //Posso decidere di non servire più il client ed uscire
  if(FLAG_TERM == 1)
  {
    printf("L'operazione del client non sarà soddisfatta\n");
    writeToStream(server->ds_clientSocket, &acc, sizeof(packet_accept));
    return 1;
  }
  
  if(inoper.oper == 1 && priv == USER_RDWR)
  {
    //Scrivi su file la nuova entry
    if( Book_insert(server->addressBook, (Entry*)&inoper.ent) == 0)
      acc.byteinfo = 1; 
    
    //acc.byteinfo sarà 0 oppure 1
    if( writeToStream(server->ds_clientSocket, &acc, sizeof(packet_accept)) < sizeof(packet_accept))
    {
      if(errno == EPIPE)
	printf("L'altro capo della rete potrebbe essere irraggiungibile per la scrittura\n");
    
      printf("Problemi durante la scrittura da socket: %s\n", strerror(errno));
      return 1;
    }
  }
  
  if(inoper.oper == 2)
  {
    int size, cont = 0, i = 0;
    AddressBook_EntryIterator iter;
    packet_entry* first;   
    size = Book_select(server->addressBook, inoper.ent.name, inoper.ent.surname, &iter);
    
    first = malloc(size * sizeof(Entry));
    if(first == 0)
    {
      printf("Memoria insufficente\n");
      return 1;
    }
    
    while(i < size)
    {
      Entry temp;
      memset(&temp, 0, sizeof(Entry));
      Book_selectNext(server->addressBook, &iter, &temp);
      if( strcmp(temp.name, inoper.ent.name) == 0 && strcmp(temp.surname, inoper.ent.surname) == 0)
      {
	memcpy(&first[cont], &temp, sizeof(packet_entry));
	cont++;
      }
      i++;
    }
    
    //Invio in numero di nominativi trovati
    writeToStream(server->ds_clientSocket, &cont, sizeof(int));
    
    //Inivio i nominativi
    if( writeToStream(server->ds_clientSocket, first, sizeof(packet_entry)*cont) < sizeof(packet_entry)*size)
    {
      if(errno == EPIPE)
	printf("L'altro capo della rete potrebbe essere irraggiungibile per la scrittura di una risposta\n");
    
      printf("Problemi durante la scrittura da socket: %s\n", strerror(errno));
      free(first);
      return 1;
    }
    
    free(first);
  }
  
  printf("Client servito\n");
  
  return 0;
  
}

void Server_close(Server* server)
{
  struct sigaction exitAction;
  struct sigaction closePipeAction;
  
  if(server->ds_clientSocket != 0)
    close(server->ds_clientSocket);
  
  if(server->ds_socket != 0)
    close(server->ds_socket);
  
  if(server->addressBook != 0)
    Book_close(server->addressBook);
  
  if(server->userBook != 0)
    UserBook_close(server->userBook);
  
  free(server);
  
  sigemptyset(&exitAction.sa_mask);
  sigemptyset(&closePipeAction.sa_mask);
  exitAction.sa_flags = 0;
  closePipeAction.sa_flags = 0;
  
  closePipeAction.sa_handler = SIG_DFL;
  if(sigaction(SIGPIPE, &closePipeAction, NULL) == -1)
    fprintf(stderr, "Problemi nell'installare un gestore per il segnale SIGPIPE: %s\n", strerror(errno));

  exitAction.sa_handler = SIG_DFL;
  if(sigaction(SIGINT, &exitAction, NULL) == -1 || sigaction(SIGQUIT, &exitAction, NULL) == -1 || sigaction(SIGTERM, &exitAction, NULL) == -1)
    fprintf(stderr, "Problemi nell'installare un gestore per i tre segnali di uscita: %s\n", strerror(errno));
}