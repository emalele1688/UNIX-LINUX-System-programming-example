#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>

#include "Server.h"

#define DEFAULT_PORTN	12000


Server* server;
UserBook* ubook;
AddressBook* abook;

//Se durante l'esecuzione avviene un segnale di eccezione senza ritorno, signal_exception si preoccupa di deallocare le strutture e chiudere il processo
void signal_exception(int sig, siginfo_t* sinfo, void* context)
{
  psignal(sig, "Un errore ha causato il segnale: ");
  printf("Eventuale locazione di memoria che ha causato l'errore %p\n", sinfo->si_addr);
  printf("Il programma sta per termianre, chiusura in corso\n");
  
  //Se il server è già stato inizializzato, lui stesso dealloca le strutture UserBook e AddressBook.
  Server_close(server);
  exit(1);
}

//Se durante l'esecuzione avviene un segnale di eccezione senza ritorno, signal_exception si preoccupa di deallocare le strutture e chiudere il processo
void signal_exit(int sig)
{
  Server_close(server);
  exit(1);
}

void printHelp(void)
{
  printf("server [opzioni registro utenti] [opzioni rubrica] [opzioni varie] [numero di porto]:\n");
  printf("\t-nu 'name': Crea nuovo registro utenti di nome 'name'. Chiede in input di inserire gli utenti.\n");
  printf("\t-lu 'name': Carica un registro utenti di nome 'name'. Chiede in input di inserire eventuali nuovi utenti.\n");
  printf("\t-nb 'name': Crea una nuova rubrica di nome 'name'.\n");
  printf("\t-lb 'name': carica una nuova rubrica di nome 'name'.\n");
  printf("\t-pu: opzione varia, stampa gli utenti nello UserBook.\n");
  printf("\t Il numero di porto è facoltativo\n");
}

int main(int argn, char* args[])
{  
  int ret = 0;
  struct sigaction exceptionSignal;
  struct sigaction exitAction;
  
  /* bit 1: 0 nuovo UserBook, 1 carica UserBook
   * bit 2: 0 nuovo AddressBook, 1 carica AddressBook
   */
  char bitarg = 0;
  
  if(argn < 5)
  {
    printHelp();
    return 0;
  }
  
  // Prendo delle scelte a seconda dell'input
  if(strcmp(args[1], "-nu") == 0)
    bitarg |= 0;
  else if(strcmp(args[1], "-lu") == 0)
    bitarg |= 1;
  else
  {
    printHelp();
    return 0;
  }
  
  if(strcmp(args[3], "-nb") == 0)
    bitarg |= 0;
  else if(strcmp(args[3], "-lb") == 0)
    bitarg |= 2;
  else
  {
    printHelp();
    return 0;
  } 
  
  server = 0;
  ubook = 0;
  abook = 0;
  
  /* Imposto i segnali nella fase di inizializzazione */
  
  signal(SIGUSR1, SIG_IGN);
  signal(SIGUSR2, SIG_IGN);
  signal(SIGCHLD, SIG_IGN);
  //SIGPIPE è momentaneamente ignorata, il server si occuperà più avanti di installare l'apposito gestore quando serve
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
  //Terminazione valida solo nalla routine Main
  sigaddset(&exitAction.sa_mask, SIGINT);
  sigaddset(&exitAction.sa_mask, SIGQUIT);
  sigaddset(&exitAction.sa_mask, SIGTERM);
  exitAction.sa_flags = 0;
  exitAction.sa_handler = signal_exit;
  ret |= sigaction(SIGINT, &exitAction, NULL);
  ret |= sigaction(SIGTERM, &exitAction, NULL);
  ret |= sigaction(SIGQUIT, &exitAction, NULL);
  if(ret == -1)
  {
    printf("Problemi nel installare i gestori di segnale di errore\n");
    return 1;
  }
  
  //INIZIALIZZO IL SERVER
  printf("Inizializzazione Server su porta %hu\n", DEFAULT_PORTN);
  server = Server_initServer(DEFAULT_PORTN);
  if(server == 0)
  {
    printf("Server non inizializzato\n");
    Server_close(server);
    return 1;
  }
  
  /* INIZIALIZZO LA STRUTTURA UserBook
   * possono essere create o caricate da file a seconda della scelta fatta in input
   */
  if((bitarg & 1) == 0)
  {
    printf("Creazione nuovo UserBook\n");
    if( (ubook = UserBook_newBook(args[2])) == 0)
      return 1;
  }
  else if((bitarg & 1) == 1)
  {
    printf("Apertura UserBook %s\n", args[2]);
    if( (ubook = UserBook_loadBook(args[2])) == 0)
      return 1;
  }
  
  printf("UserBook aperto con successo\n");
  
  //Stampa gli utenti se riciesto
  if(argn > 5 && strcmp(args[5], "-pu") == 0)
    UserBook_printAllUsers(ubook);
  
  /* INIZIALIZZO LA STRUTTURA AddressBook
   * possono essere create o caricate da file a seconda della scelta fatta in input
   */
  if((bitarg & 2) == 0)
  {
    printf("Creazione nuovo AddressBook\n");
    if( (abook = Book_newBook(args[4])) == 0)
    {
      UserBook_close(ubook);
      return 1;
    }
  }
  else if((bitarg & 2) == 2)
  {
    printf("Apertura AddressBook %s\n", args[4]);
    if( (abook = Book_loadBook(args[4])) == 0)
    {
      UserBook_close(ubook);
      return 1;
    }
  }
  
  printf("AddressBook aperto con successo\n");
  
  Server_setUserBook(server, ubook);
  Server_setAddressBook(server, abook);
  
  printf("Esecuzione server\n");
  Server_execute(server);
  
  Server_close(server);
  
  return 0;
}
