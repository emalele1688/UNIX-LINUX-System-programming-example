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

#include "Streamer.h"
#include "ServerProtocol.h"


#define DEFAULT_PORTN	12000
#define USER_RD		49
#define USER_RDWR	50


static int FLAG_TERM;
static int FLAG_BPIPE;

static int actsocket;

void signal_exception(int sig, siginfo_t* sinfo, void* context)
{
  psignal(sig, "Un errore ha causato il segnale: ");
  printf("Eventuale locazione di memoria che ha causato l'errore %p\n", sinfo->si_addr);
  printf("Il programma sta per termianre, chiusura in corso\n");
    
  //Se il server è già stato inizializzato, lui stesso dealloca le strutture UserBook e AddressBook.
  if(actsocket == 0)
  close(actsocket);
  exit(1);
}

void Client_term(int sig)
{
  printf("Termination\n");
  FLAG_TERM = 1;
}

void Client_brokenPipe(int sig)
{
  printf("Broken pipe\n");
  FLAG_BPIPE = 1;
}

int Client_service(int ds_socket)
{
  packet_login inlog;
  packet_entry outent;
  packet_operation inoper;
  packet_accept acc;
  int scelta = 0, ret;
  char priv;
  
  memset(&inlog, 0, sizeof(packet_login));
  memset(&outent, 0, sizeof(packet_entry));
  memset(&inoper, 0, sizeof(packet_operation));
  memset(&acc, 0, sizeof(packet_accept));
  
  printf("User name: ");
  scanf("%s", inlog.user_name);
  while(getchar() != '\n');
  printf("Password: ");
  scanf("%s", inlog.user_pass);
  while(getchar() != '\n');
  
  //Se non ho ancora effettuato il Login al Server faccio in tempo a termianare
  if(FLAG_TERM == 1)
    return 1;
  
  //Invia login
  if( writeToStream(ds_socket, &inlog, sizeof(packet_login)) < sizeof(packet_login))
  {
    if(errno == EPIPE)
      printf("L'altro capo della rete potrebbe essere irraggiungibile per la scrittura\n");
    
    printf("Problemi durante la scrittura sul socket: %s\n", strerror(errno));
    return 1;
  }
  
  //Ricevi la risposta
  if( (ret = readToStream(ds_socket, &acc, sizeof(packet_accept))) < sizeof(packet_accept))
  {
    if(ret == 0)
      printf("il server potrebbe aver chiuso la connessione\n");
    else
      printf("Problemi durante la lettura da socket: %s\n", strerror(errno));
    
    return 1;
  }
  
  //Se il server ha rifiutato il Login poichè il nome utente o password sono errati
  if(acc.byteinfo == 0)
  {
    printf("Utente non registrato o password sbagliata\n");
    return 1;
  }
  
  //Login accettato - procedi
  priv = acc.byteinfo;
  memset(&acc, 0, sizeof(packet_accept));
  
  if(priv == USER_RD)
    printf("L'utente ha privileggi di sola lettura\n");
  else if(priv == USER_RDWR)
    printf("L'utente ha privileggi di lettura e scrittura\n");
  
  while(scelta != 1 && scelta != 2)
  {
    printf("Quale operazione desideri?\n\t1:inserisci nuovo nominativo\n\t2:trova nominativo\n");
    scanf("%d", &scelta);
    
    //Posso decidere di terminare. Il Server dall'altro capo è in read quindi uscirebbe anche lui.
    if(FLAG_TERM == 1)
      return 1;
    
    if(scelta == 1 && priv != USER_RDWR)
    {
      printf("Non si hanno privileggi per la scrittura\n");
      scelta = 0;
      continue;
    }
  }
  
  if(scelta == 1)
  {
    //Imposto operazione
    inoper.oper = 1;
    
    printf("Inserisci le nuove entry. Operazione terminabile alla fine con CTRL+C\n");
    while(getchar() != '\n');
    printf("Inserisci nome\n");
    fgets(inoper.ent.name, FIELD_1, stdin);
    printf("Inserisci cognome\n");
    fgets(inoper.ent.surname, FIELD_1, stdin);
    printf("Inserisci indirizzo\n");
    fgets(inoper.ent.address, FIELD_1, stdin);
    printf("Inserisci telefono\n");
    fgets(inoper.ent.telephone, FIELD_2, stdin);
    
    //Posso decidere di terminare. Il Server dall'altro capo è in read quindi uscirebbe anche lui.
    if(FLAG_TERM == 1)
    {
      printf("Le entry non saranno registrate\n");
      return 1;
    }
    
    if( writeToStream(ds_socket, &inoper, sizeof(packet_operation)) < sizeof(packet_operation))
    {
      if(errno == EPIPE)
	printf("L'altro capo della rete potrebbe essere irraggiungibile per la scrittura\n");
      
      printf("Problemi durante la scrittura sul socket: %s\n", strerror(errno));
      return 1;
    }
    
    //Attendi risposta
    if( (ret = readToStream(ds_socket, &acc, sizeof(packet_accept))) < sizeof(packet_accept))
    {
      if(ret == 0)
	printf("Il server potrebbe aver chiuso la connessione. Nessun nominativo potrebbe essere stato inserito\n");
      else
	printf("Problemi durante la lettura da socket: %s\n", strerror(errno));
    
      return 1;
    }
    
    //Se l'operazione è rifiutata dal server (perchè non si hanno privileggi o per errore)
    if(acc.byteinfo == 0)
    {
      printf("L'operazione è stata rifiutata dal Server\n");
      return 1;
    }
    else
    {
      printf("L'operazione è stata accetta dal Server\n");
      return 0;
    }
  }
  else if(scelta == 2)
  {
    packet_entry* first;
    int cont = 0, i = 0;
    
    //Imposto operazione
    inoper.oper = 2;
    
    printf("Inserisci il nominativo cercato. Operazione terminabile alla fine con CTRL+C\n");
    while(getchar() != '\n');
    printf("Inserisci nome\n");
    fgets(inoper.ent.name, FIELD_1, stdin);
    printf("Inserisci cognome\n");
    fgets(inoper.ent.surname, FIELD_1, stdin);
    
    //Posso decidere di terminare. Il Server dall'altro capo è in read quindi uscirebbe anche lui.
    if(FLAG_TERM == 1)
    {
      printf("Le entry non saranno registrate\n");
      return 1;
    }
    
    if( writeToStream(ds_socket, &inoper, sizeof(packet_operation)) < sizeof(packet_operation))
    {
      if(errno == EPIPE)
	printf("L'altro capo della rete potrebbe essere irraggiungibile per la scrittura\n");
      
      printf("Problemi durante la scrittura sul socket: %s\n", strerror(errno));
      return 1;
    }
    
    //Attendo numero di entry
    readToStream(ds_socket, &cont, sizeof(int));
    
    if(cont == 0)
    {
      printf("Nessun nominativo trovato\n");
      return 0;
    }
    
    printf("%d nominativi trovati\n", cont);
    
    first = malloc(sizeof(packet_entry) * cont);
    if(first == 0)
    {
      printf("Memoria insufficente\n");
      return 1;
    }
    
    if( (ret = readToStream(ds_socket, first, sizeof(packet_entry)*cont)) < sizeof(packet_entry)*cont)
    {
      if(ret == 0)
	printf("Il server potrebbe aver chiuso la connessione.\n");
      else
	printf("Problemi durante la lettura da socket: %s\n", strerror(errno));
      
      free(first);
      return 1;
    }
    
    while(i < cont)
    {
      printf("nome: %s", first[i].name);
      printf("cognome: %s", first[i].surname);
      printf("indirizzo: %s", first[i].address);
      printf("telefono: %s", first[i].telephone);
      i++;
    }
    
    free(first);
  }
  
  return 0;
}

int Client_execute(const char* ipAddress, uint16_t port)
{
  sigset_t blockedSig;
  struct sigaction closePipeAction;
  int ds_socket, ret;
  struct sockaddr_in serveraddr;
    
  ds_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  if(ds_socket == -1)
  {
    printf("Impossibile aprire un socket: %s\n", strerror(errno));
    return 1;
  }
  
  actsocket = ds_socket;
  
  serveraddr.sin_family = AF_INET;
  serveraddr.sin_port = htons(port);
  if(ipAddress != NULL) //Se NULL il processo Server è in local host
    serveraddr.sin_addr.s_addr = inet_addr(ipAddress);
  if(serveraddr.sin_addr.s_addr == INADDR_NONE)
  {
    printf("Indirizzo IP del server errato\n");
    return 1;
  }
  
  if(FLAG_TERM == 1)
  {
    close(ds_socket);
    return 1;
  }
  
  //Il client ha porta effimera
  
  while( (ret = connect(ds_socket, (struct sockaddr*)(&serveraddr), sizeof(struct sockaddr_in))) == -1) 
  {
    if(ret == -1)
    {
      if(errno == EINTR)
	continue;
      else
      {
	printf("Connessione fallita: %s\n", strerror(errno));
	close(ds_socket);
	return 1;
      }
    }
  }
  
  printf("Connesso con successo\n");
  
  sigemptyset(&closePipeAction.sa_mask);
  closePipeAction.sa_flags = 0;
  closePipeAction.sa_handler = Client_brokenPipe;
  if(sigaction(SIGPIPE, &closePipeAction, NULL) == -1)
  {
    fprintf(stderr, "Problemi nell'installare un gestore per il segnale SIGPIPE: %s\n", strerror(errno));
    close(ds_socket);
    return 0;
  }

  if(FLAG_TERM == 1)
  {
    close(ds_socket);
    return 1;
  }
  
  //Blocco il segnale di SIGTERM e SIGTSTP durante l'interazione con il server
  sigaddset(&blockedSig, SIGTERM);
  sigaddset(&blockedSig, SIGTSTP);
  sigprocmask(SIG_SETMASK, &blockedSig, NULL);
  
  Client_service(ds_socket);
  
  sigemptyset(&blockedSig);
  sigprocmask(SIG_SETMASK, &blockedSig, NULL);
  
  printf("Disconnessione in corso\n");
  
  closePipeAction.sa_flags = 0;
  closePipeAction.sa_handler = SIG_IGN;
  if(sigaction(SIGPIPE, &closePipeAction, NULL) == -1)
    fprintf(stderr, "Per un problema il gestore SIGPIPE non sarà ignorato: %s\n", strerror(errno));
  
  close(ds_socket);
  
  return 0;
}

int main(int argn, char* args[])
{
  if(argn < 2)
  {
    printf("client [Server IP]\nPer server localhost mettere 0\n");
    return 0;
  }
  
  int ret = 0;
  struct sigaction exceptionSignal;
  struct sigaction exitAction;
  
  FLAG_TERM = 0;
  FLAG_BPIPE = 0;
  actsocket = 0;
  
  signal(SIGUSR1, SIG_IGN);
  signal(SIGUSR2, SIG_IGN);
  signal(SIGCHLD, SIG_IGN);
  signal(SIGPIPE, SIG_IGN);
  
  sigemptyset(&exceptionSignal.sa_mask);
  sigaddset(&exceptionSignal.sa_mask, SIGSEGV);
  sigaddset(&exceptionSignal.sa_mask, SIGFPE);
  sigaddset(&exceptionSignal.sa_mask, SIGILL);
  sigaddset(&exceptionSignal.sa_mask, SIGBUS);
  sigaddset(&exceptionSignal.sa_mask, SIGABRT);
  sigaddset(&exceptionSignal.sa_mask, SIGSYS);
  exceptionSignal.sa_flags = SA_SIGINFO;
  exceptionSignal.sa_sigaction = signal_exception;
  
  ret |= sigaction(SIGSEGV, &exceptionSignal, NULL);
  ret |= sigaction(SIGFPE, &exceptionSignal, NULL);
  ret |= sigaction(SIGILL, &exceptionSignal, NULL);
  ret |= sigaction(SIGBUS, &exceptionSignal, NULL);
  ret |= sigaction(SIGABRT, &exceptionSignal, NULL);
  ret |= sigaction(SIGSYS, &exceptionSignal, NULL);
  if(ret == -1)
  {
    printf("Problemi nel installare i gestori di segnale di errore\n");
    return 1;
  }
  
  sigemptyset(&exitAction.sa_mask);
  sigaddset(&exitAction.sa_mask, SIGINT);
  sigaddset(&exitAction.sa_mask, SIGQUIT);
  sigaddset(&exitAction.sa_mask, SIGTERM);
  exitAction.sa_flags = 0;
  exitAction.sa_handler = Client_term;
  if(sigaction(SIGINT, &exitAction, NULL) == -1 || sigaction(SIGQUIT, &exitAction, NULL) == -1 || sigaction(SIGTERM, &exitAction, NULL) == -1)
  {
    fprintf(stderr, "Problemi nell'installare un gestore per i tre segnali di uscita: %s\n", strerror(errno));
    return 0;
  }

  //Se chiesta terminazione prima di iniziare ad eseguire il client, esci
  if(FLAG_TERM == 1)
    return 0;
  
  if(args[1] == 0)
    Client_execute(NULL, DEFAULT_PORTN); //Localhost
  else
    Client_execute(args[1], DEFAULT_PORTN); //Server remoto
  
  return 0;
}