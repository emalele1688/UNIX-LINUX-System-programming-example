#include <stdio.h>
#include <signal.h>
#include <sys/types.h>
#include <fcntl.h>

typedef struct
{
  char fiforesponse[6];
  char messaggio[15];
} 
messaggio;

int pidcl;
int fd0, fd1;

void nessuna_risposta()
{
  printf("Tempo scaduto! Nessuna risposta da parte del client\n");
  gestione_terminazione_server();
  exit(0);
}

void gestione_terminazione_server()
{
  //Stampa il segnale ricevuto (memorizzato in sys_signal presente in signal.h
  char sig[20];
  psignal(sys_siglist, sig);
  
  printf("Segnale ricevuto: %s\n", sig);
  printf("Chiusura del Server in corso\n");
  unlink("serv");
  close(fd0);
  close(fd1);
  printf("Shutdown del server completato\n");
  
  //Se il client è connesso informagli della mia terminazione (segnale generico SIGUSR1)
  if(pidcl != 0)
  {
    printf("Avviso di termnazione al client\n");
    kill(pidcl, SIGUSR1);
  }
}

int main(int arg, char* args[])
{
  int ret;
  messaggio mess;
  pidcl = 0;
  
  alarm(15);
  //Da alarm
  signal(SIGALRM, nessuna_risposta);
  //Quando il processo sente una sigterm il processo conclude e il S.O. lo termina (inviato dalla kill)
  signal(SIGTERM, gestione_terminazione_server);
  //Ctrl+C
  signal(SIGINT, gestione_terminazione_server);
  //Errore di segmentazione
  signal(SIGSEGV, gestione_terminazione_server);
  
  ret = mkfifo("serv", O_CREAT|O_EXCL|0666);
  if(ret == -1)
  {
    printf("Impossibile creare il canale di comunicazione verso il client\n");
    unlink("serv");
    exit(1);
  }
  
  fd0 = open("serv", O_RDWR);
  if(fd0 == -1)
  {
    printf("Impossibile aprire il canale di comunicazione verso il client\n");
    unlink("serv");
    exit(1);
  }
  
  //Leggo il pid del client
  read(fd0, &pidcl, sizeof(int));
  printf("Connesso: %u\n", pidcl);
  //Leggo il messaggio
  read(fd0, &mess, sizeof(messaggio));
  
  printf("Messaggio del client: %s\nRisposta:\n", mess.messaggio);
  fgets(mess.messaggio, 15, stdin);
  
  //Invio messaggio al client------------------------------------------------------------------------------
  fd1 = open(mess.fiforesponse, O_RDWR);
  if(fd1 == -1)
  {
    printf("Impossibile aprire il canale di comunicazione del client, il client è avviato?\n");
    unlink("serv");
    exit(1);
  }
  
  write(fd1, &mess, sizeof(messaggio));
  
  gestione_terminazione_server();
  
  return 0;
}
