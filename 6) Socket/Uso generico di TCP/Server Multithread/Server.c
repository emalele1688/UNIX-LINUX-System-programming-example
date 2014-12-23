/*Ricevi una connessione su TCP, avvia un thread per gestirla, il server nel frattempo stampa l'ip del client e torna in ascolto di nuove connessioni. Dopo 60 secondi di inattività termina
 */

#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <errno.h>

#include "TrasfererMan.h"

#define PORTN 1912
#define TIMEOUT 10
#define MAXCONNECT 3
#define MESGSIZE 256

sigset_t sig;
int STOP_INT = 0;
int STOP_TIMER = 0;

void connessione_senza_lettore(void)
{
  printf("[pid %u]: La connessione è stata chiusa sull'altro lato\n", pthread_self());
}

void server_interrupt(void)
{
  STOP_INT = 1;
}

/*Serve una richiesta client
 * Quattro messaggi al client e attende una risposta ... finchè il client non chiude la connessione
 */
void* service(void* dss)
{
  int ret, i = 0;
  int ds_sockin = *((int*)dss);
  
  struct sigaction sigpipe;
  sigpipe.sa_handler = (void*)&connessione_senza_lettore;
  sigpipe.sa_mask = sig;
  sigaction(SIGPIPE, &sigpipe, NULL);
  
  sigemptyset(&sig);
  sigaddset(&sig, SIGINT);
  pthread_sigmask(SIG_BLOCK, &sig, NULL);

  char buffer[MESGSIZE];
  size_t receiveByte;
  
  while(1)
  {
    while(i < 4)
    {
      printf("Invia Messaggio %d:\n", i);
      fgets(buffer, MESGSIZE, stdin);
      usleep(10);
      ret = sendData(ds_sockin, buffer, MESGSIZE);
      
      if(ret == -1)
      {
	printf("Errore in scrittura sul socket\n");
	close(ds_sockin);
	pthread_exit(0);
      }
      i++;
    }
    i = 0;
       
    receiveByte = receiveData(ds_sockin, buffer, MESGSIZE);
    if(receiveByte == -1)
    {
      printf("Errore in lettura\n");
      close(ds_sockin);
      pthread_exit(0);
    }
    else if(receiveByte == 0)
    {
      printf("Il client ha chiuso la connessione\n");
      close(ds_sockin);
      pthread_exit(0);
    }
    
    if(strncmp(buffer, "quit", 4) == 0)
    {
      printf("Il client ha dato quit\n");
      break;
    }
      
    printf("Client: %s\n", buffer);
  }
  
  close(ds_sockin);
  pthread_exit(0);
}

void gestione_timeout(void)
{
  STOP_TIMER = 1;
}

void gestione_interrupt(void)
{
  printf("Uscita richiesta da terminale\n");
  STOP_INT = 1;
}

void server(unsigned short int porto)
{
  int ds_sock;
  //Indirizzo server e indirizzo del client connesso
  struct sockaddr_in sraddr, claddr;
  memset(&claddr, 0x0, sizeof(claddr));
  memset(&sraddr, 0x0, sizeof(sraddr));
  
  //Vuole che il server resta su 60 secondi, abilito una SIGALRM con un set di segnali-------
  struct sigaction sigtimer;
  struct sigaction sigint;
  memset(&sigtimer, 0, sizeof(sigtimer));
  memset(&sigint, 0, sizeof(sigint));
  
  sigemptyset(&sig);
  sigaddset(&sig, SIGINT);
  sigaddset(&sig, SIGALRM);
  sigaddset(&sig, SIGPIPE);
  sigtimer.sa_mask = sig;
  sigtimer.sa_handler = (void*)&gestione_timeout;
  sigaction(SIGALRM, &sigtimer, NULL);
  sigint.sa_mask = sig;
  sigint.sa_handler = (void*)&gestione_interrupt;
  sigaction(SIGINT, &sigint, NULL);

  alarm(TIMEOUT);
  
  //Il segnale SIGINT deve interrompere le system call bloccanti
  siginterrupt(SIGINT, 1);
  
  //Se un figlio termina, deallocalo
  signal(SIGCHLD, SIG_IGN);
  
  //Creo il socket con i relativi indirizzi-------------------------------------------------
  ds_sock = socket(AF_INET, SOCK_STREAM, 0);
  if(ds_sock == -1)
  {
    printf("Eèèrh!\n");
    exit(1);
  }
  printf("Server Socket index %d\n", ds_sock);
  
  sraddr.sin_family = AF_INET;
  sraddr.sin_port = htons(porto);
  sraddr.sin_addr.s_addr = htonl(INADDR_ANY);
  
  if(bind(ds_sock, (void*)&sraddr, sizeof(sraddr)) == -1)
  {
    printf("Impossibile aprire un socket su tale indirizzo %s\n", strerror(errno));
    close(ds_sock);
    exit(1);
  }
  
  //Inizio la procedura client/server-----------------------------------------------------------
  int ds_sockin, ret;
  
  listen(ds_sock, MAXCONNECT);
  
  while(1)
  {
    printf("Attendo connessioni\n");
    //ATTENTO A LENGTH CAZZO!!!
    //lenght deve essere inizializzato con la quantità di byte che vogliamo indietro dal claddr. Poi lenght torna la dimensione effettivamente inizializzata 
    socklen_t lenght = sizeof(struct sockaddr_in);
    if( (ds_sockin = accept(ds_sock, (struct sockaddr *)&claddr, &lenght)) == -1)
    {
      //EINTR un segnale ha sbloccato la syscall bloccante (accept in tal caso)
      if(errno == EINTR && STOP_TIMER == 1)
      {
	printf("Timer scaduto\n");
	break;
      }
      else if(errno == EINTR && STOP_INT == 1)
      {
	printf("Termino il server!");
	break;
      }
      else
	printf("Un client ha tentato di connettersi ma non c'è riuscito! Oppure è successo qualcosa al processo server, torno in ascolto.\n");
      //Nessuno si è connesso, torno sopra il ciclo e se Stop è 0 esco
      continue;
    }
    
    //Cancello l'alarm precedente poichè è arrivata una connessione
    alarm(0);
    
    pthread_t tid;
    ret = pthread_create(&tid, NULL, service, (void*)&ds_sockin);
    if(ret == -1)
    {
      printf("Impossibile avviare un thread in questo momento\n");
      write(ds_sockin, "ERROR", 5);
      close(ds_sockin);
    }
    pthread_detach(tid);
     
    //Stampo l'indirizzo del client appena connesso e torno in ascolto  
    printf("Indirizzo del client connesso: %s:%i\n", inet_ntoa(claddr.sin_addr), ntohs(claddr.sin_port));
  }
  
  printf("Attendo la terminazione di eventuali connessioni\n");
  //Waitpid(0, status, flag per i gruppi) attende la terminazione di tutti i processi figli
  waitpid(NULL, NULL, NULL);
  
  close(ds_sock);
  
}

int main(int s, char* arg[])
{
  printf("Avvio server in corso\n");
  server(PORTN);
  printf("Terminato!\n");
  return 0;
}
