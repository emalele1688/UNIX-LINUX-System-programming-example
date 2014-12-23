#include <stdio.h>
#include <string.h>
#include <fcntl.h>

typedef struct
{
  long type;
  char fifo_response[20];
} request;

int main(int argc, char* argv)
{
  //Risposta del Server
  char response[20];
  //Descrittore per la FIFO aperta dal Server
  int fds;
  //Descrittore per la FIFO che apre il client per leggere la risposta del Server
  int fdc;
  //Richiesta per inviare il nome della FIFO aperta dal client al Server
  request r;
  
  printf("Servizio da richiedere al Server\n");
  fgets(r.fifo_response, 20, stdin);
  int responseSize = strlen(r.fifo_response) + 1;
  r.fifo_response[responseSize] = '\0';
  
  //Creo una FIFO dove il Server scriverà la risposta
  if( mkfifo(r.fifo_response, O_CREAT|0666) == -1 )
  {
    printf("Errore nel creare la FIFO\n");
    exit(-1);
  }
  
  //Apro la FIFO del Server
  fds = open("serv", O_WRONLY);
  if(fds == -1)
  {
    printf("Servizio non disponibile\n");
    unlink(r.fifo_response);
    exit(-1);
  }
  
  //Scrivo nella FIFO del Server la richiesta da me fatta
  write(fds, &r, sizeof(request));
  //Chiudo la FIFO del Server
  close(fds);
  
  //Apro la FIFO del client (quella creata da me
  fdc = open(r.fifo_response, O_RDWR);
  //Leggo cosa il server ha scritto nella mia FIFO
  read(fdc, response, 20);
  printf("risposta: %s", response);
  
  //Chiudo la mia FIFO
  close(fdc);
  unlink(r.fifo_response);
  
}