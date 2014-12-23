#include <stdio.h>
#include <fcntl.h>

typedef struct
{
  long type;
  char fifo_response[20];
} request;

int main(int argc, char* argv)
{
  //risposta del Server al Client
  char serverResponse[20];
  int pid;
  //Descrittore del Server
  int fds;
  //Descrittore del Client
  int fdc;
  //Memorizzo la richiesta del client
  request r;
  
  //Creo una FIFO per permettere ai processi di parlare con me
  if( mkfifo("serv", O_CREAT|0666) == -1 )
  {
    printf("Errore nel creare la FIFO\n");
    exit(-1);
  }
  
  //Apro la mia FIFO
  fds = open("serv", O_RDWR);
  
  int ret = 0;
  while(1)
  {
    //Leggo sulla mia FIFO ciò che il client richiede (r.fifo_response conterrà il nome della FIFO del Client
    ret = read(fds, &r, sizeof(request));
    
    if(ret != 0)
    {
      //Apro l'esecuzione della richiesta su un nuovo processo
      pid = fork();
      if(pid == 0)
      {
	printf("Richiesto un servizio su fifo %s\n", r.fifo_response);
	//Apro la FIFO del Client per scrivere la risposta
	fdc = open(r.fifo_response, O_WRONLY);
	//Simulo un processo Server
	sleep(2);
	printf("Messaggio da inviare al server\n");
	fgets(serverResponse, 20, stdin);
	//Scrivo (o invio) il messaggio al Client mediante la sua fifo
	ret = write(fdc, serverResponse, 20);
	//Chiudo la sua FIFO
	ret = close(fdc);
	exit(0);
      }
    }
  }
}